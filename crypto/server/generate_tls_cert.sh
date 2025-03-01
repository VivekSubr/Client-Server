set -x

#clean
rm -f *.pem *.cert *.key *.csr *.crt

#generate private key
#This key is a 1024 bit RSA key which is encrypted using Triple-DES and stored in a PEM format so 
#that it is readable as ASCII text.
#openssl genrsa -out private.pem 1024

#create public key
#openssl rsa -in private.pem -outform PEM -pubout -out public.pem

#create X509 cert
#X.509 certificate is a standard way to distribute public keys, signed by a certificate authority...
#here, this is self signed
#openssl req -new -x509 -key private.pem -out ca.cert -days 365 -config server.req 

#generate ca private key,
openssl genpkey -algorithm RSA -out ca.key -config server.req 

#generate ca cert
openssl req -new -x509 -key ca.key -out ca.cert -days 365 -config server.req 

#generate user private key
openssl genpkey -algorithm RSA -out user.key

#create certificate signing request using user key
openssl req -new -key user.key -out user.csr -config server.req

#create and sign cert using ca cert
openssl x509 -req -in user.csr -CA ca.cert -CAkey ca.key -CAcreateserial -out user.crt -days 365 -sha256 