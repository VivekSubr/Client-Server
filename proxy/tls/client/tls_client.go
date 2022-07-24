package main

import (
    "log"
    "os"
    "crypto/tls"
    "crypto/x509"
)

var CRT = "../server/publickey.cer"
var KEY = "../server/private.pem"

func main() {
    log.SetFlags(log.Lshortfile)
    roots := x509.NewCertPool()
    rootPEM, err := os.ReadFile(CRT)
	if err != nil {
		panic(err)
	}

	ok := roots.AppendCertsFromPEM([]byte(rootPEM))
	if !ok {
		panic("failed to parse root certificate")
	} 
	
    conf := &tls.Config{ RootCAs: roots}
    conn, err := tls.Dial("tcp", "127.0.0.1:9443", conf)
    if err != nil {
        log.Println(err)
        return
    }
    defer conn.Close()

    n, err := conn.Write([]byte("hello\n"))
    if err != nil {
        log.Println(n, err)
        return
    }

    buf := make([]byte, 100)
    n, err = conn.Read(buf)
    if err != nil {
        log.Println(n, err)
        return
    }

    println(string(buf[:n]))
}