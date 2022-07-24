set -x

#clean
rm -f *.pem *.cer

#generate private key
#This key is a 1024 bit RSA key which is encrypted using Triple-DES and stored in a PEM format so 
#that it is readable as ASCII text.
openssl genrsa -out private.pem 1024

#create public key
openssl rsa -in private.pem -outform PEM -pubout -out public.pem

#create X509 cert
#X.509 certificate is a standard way to distribute public keys, signed by a certificate authority...
#here, this is self signed
openssl req -new -x509 -key private.pem -out publickey.cer -days 365 -config server.req