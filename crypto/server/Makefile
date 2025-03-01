build:
	go build

debug: #change the args
	dlv debug tls_server.go -- -ip=127.0.0.1 -port=1447 -crt=/ca/tls-crt -key=/ca/tls-key

test:
	openssl s_client 127.0.0.1:1447

clean:
	go clean
	rm -f *.pem *.cert *.key *.csr *.crt
