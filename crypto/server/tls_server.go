package main

import (
    "fmt"
    "log"
    "flag"
    "crypto/tls"
    "crypto/x509"
    "net"
    "bufio"
    "io/ioutil"
    //"time"
)

func main() {
    ipPtr       := flag.String("ip", "0.0.0.0",  "ip server listening to")
    portPtr     := flag.String("port", "3000",   "port server listening to")
    crtPtr      := flag.String("crt", "selfSigned.cert", "X.509 certificate")
    keyPtr      := flag.String("key", "private.pem", "private key")
    caCrtPtr    := flag.String("caCrt", "", "A PEM encoded CA's certificate file")
    insecurePtr := flag.String("insecure", "false", "skip verifying cert chain")
    flag.Parse()

    fmt.Printf("listening on %s\n", *ipPtr + ":" + *portPtr)
    log.SetFlags(log.Lshortfile)

    cer, err := tls.LoadX509KeyPair(*crtPtr, *keyPtr)
    if err != nil {
        log.Println(err)
        return
    }

    insecure := false
    if *insecurePtr == "true" {
        insecure = true
    }

    var config *tls.Config
    if *caCrtPtr == "" {
        config = &tls.Config{
            Certificates: []tls.Certificate{cer},
            MinVersion:    tls.VersionTLS12,
            InsecureSkipVerify: insecure,
        }
    } else {
        fmt.Printf("Using ca cert %s\n", *caCrtPtr)
        caCert, err := ioutil.ReadFile(*caCrtPtr)
        if err != nil {
            log.Fatal(err)
        }
        caCertPool := x509.NewCertPool()
        caCertPool.AppendCertsFromPEM(caCert)

        config = &tls.Config{
            Certificates: []tls.Certificate{cer},
            RootCAs:      caCertPool,
            MinVersion:   tls.VersionTLS12,
            InsecureSkipVerify: insecure,
        }
    }
    
    printTlsConfig(config)
    ln, err := tls.Listen("tcp",  *ipPtr + ":" + *portPtr, config) 
    if err != nil {
        log.Println(err)
        return
    }
    defer ln.Close()

    for {
        conn, err := ln.Accept()
        //conn.SetReadDeadline(time.Now().Add(30*time.Second))
        if err != nil {
            log.Println(err)
            continue
        }
        go handleConnection(conn)
    }
}

func handleConnection(conn net.Conn) {
    defer conn.Close()
    r := bufio.NewReader(conn)
    for {
        msg, err := r.ReadString('\n')
        if err != nil {
            fmt.Printf("errored - %v\n", err.Error())
            continue
        }

        if(msg == "") {
            fmt.Println("empty message")
        }
        
        fmt.Println(msg)

        n, err := conn.Write([]byte("world\n"))
        if err != nil {
            log.Println(n, err)
            return
        }

        fmt.Print("Replied\n")
    }
}

func printTlsConfig(config *tls.Config) {    
    tlsVersion := ""
    switch(config.MinVersion) {
        case tls.VersionTLS11: tlsVersion = "TLS 1.1"
        case tls.VersionTLS12: tlsVersion = "TLS 1.2"
        case tls.VersionTLS13: tlsVersion = "TLS 1.3"
    }

    insecure := "false"
    if config.InsecureSkipVerify {
        insecure = "true"
    }

    fmt.Printf("TLS Config\n minVersion : %s\tInsecure:%s", tlsVersion, insecure)
}