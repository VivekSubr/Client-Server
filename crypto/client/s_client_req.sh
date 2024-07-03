set -x

#Connect to tls server using s_client: https://www.openssl.org/docs/man1.0.2/man1/openssl-s_client.html

#this shows the TLS verification of endpoint
openssl s_client -connect 127.0.0.1:3000 -CAfile ../server/selfSigned.cert 