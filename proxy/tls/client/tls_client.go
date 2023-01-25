package main

import (
    "fmt"
    "log"
    "flag"
    "os"
    "time"
    "crypto/tls"
    "crypto/x509"
)

func main() {
    ipPtr := flag.String("ip", "0.0.0.0", "ip server listening to")
    portPtr := flag.String("port", "3000", "port server listening to")
    crtPtr := flag.String("crt", "../server/publickey.cer", "public key")

    flag.Parse()
    fmt.Printf("sending to %s\n", *ipPtr + ":" + *portPtr)

    log.SetFlags(log.Lshortfile)
    roots := x509.NewCertPool()
    rootPEM, err := os.ReadFile(*crtPtr)
	if err != nil {
		panic(err)
	}

	ok := roots.AppendCertsFromPEM([]byte(rootPEM))
	if !ok {
		panic("failed to parse root certificate")
	} 
	
    conf := &tls.Config{RootCAs: roots}

    for {
        fmt.Printf("sending...\n")
        conn, err := tls.Dial("tcp",  *ipPtr + ":" + *portPtr, conf)
        if err != nil {
            log.Println(err)
            time.Sleep(2 * time.Second)
            continue
        }
        defer conn.Close()

        n, err := conn.Write([]byte("hello\n"))
        if err != nil {
            log.Println(n, err)
            time.Sleep(2 * time.Second)
            continue
        }

        buf := make([]byte, 100)
        n, err = conn.Read(buf)
        if err != nil {
            log.Println(n, err)
            return
        }

        println(string(buf[:n]))
        time.Sleep(2 * time.Second)
    }
}
