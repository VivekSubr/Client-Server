#!/bin/bash

rm -f tlsServer.log tlsClient.log
killall tlsServer

echo "starting tls server"
./server/tlsServer --caCrt=ingress-li-ca-crt --crt=ingress-li-tls-crt --key=ingress-li-tls-key & > tlsServer.log

echo "connect using tls client"
./client/tlsClient --caCrt=ingress-li-ca-crt --crt=ingress-li-tls-crt --key=ingress-li-tls-key --insecure=false

