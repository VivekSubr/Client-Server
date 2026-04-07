
#!/usr/bin/env bash
set -euxo pipefail
umask 077 #sets default file permissions to rw------- for any files created by script

# Clean
rm -f *.pem *.cert *.key *.csr *.crt *.srl *.cnf

# -------- Settings --------
CA_CN="Example Test CA"
USER_CN="Vivek User Cert"
CA_DAYS=1825
USER_DAYS=365

# -------- OpenSSL configs --------
# Base config used for both CA and user; user path won't include SAN.
cat > base.cnf <<'EOF'
[ req ]
default_bits = 4096
prompt = no
distinguished_name = dn

[ dn ]
C = IN
ST = Karnataka
L = Bengaluru
O = Example Org
OU = Engineering
CN = __CN__

# CA certificate extensions
[v3_ca]
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer
basicConstraints = critical, CA:TRUE, pathlen:0
keyUsage = critical, keyCertSign, cRLSign

# End-entity certificate extensions (NO SAN)
[v3_req_nosan]
basicConstraints = CA:FALSE
keyUsage = critical, digitalSignature, keyEncipherment
# If used only for client mTLS, you may prefer: extendedKeyUsage = clientAuth
# If needed for both: 
extendedKeyUsage = clientAuth, serverAuth
EOF

# Derive specific configs
sed "s/__CN__/${CA_CN}/" base.cnf > ca.cnf
sed "s/__CN__/${USER_CN}/" base.cnf > user.cnf

# -------- Generate CA key & self-signed cert --------
openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -out ca.key

openssl req -new -x509 -key ca.key -out ca.cert -days "${CA_DAYS}" \
  -config ca.cnf -extensions v3_ca

# -------- Generate user key & CSR (NO SAN) --------
openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -out user.key

# CSR without SAN: use req section + DN only
openssl req -new -key user.key -out user.csr -config user.cnf

# -------- Sign user cert with NO SAN --------
openssl x509 -req -in user.csr -CA ca.cert -CAkey ca.key -CAcreateserial \
  -out user.crt -days "${USER_DAYS}" -sha256 \
  -extensions v3_req_nosan -extfile user.cnf

# -------- Verify & outputs --------
cat user.crt > user.fullchain.crt
cat ca.cert >> user.fullchain.crt

openssl verify -CAfile ca.cert user.crt

ls -l ca.key ca.cert user.key user.csr user.crt user.fullchain.crt
