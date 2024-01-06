package main

import (
    "fmt"
    "log"
    "flag"
    "time"
    "crypto/tls"
    "crypto/x509"
    "io/ioutil"
)

func main() {
    ipPtr       := flag.String("ip", "0.0.0.0", "ip server listening to")
    portPtr     := flag.String("port", "3000", "port server listening to")
    crtPtr      := flag.String("crt", "../server/selfSigned.cert", "certificate")
    keyPtr      := flag.String("key", "../server/private.pem", "PEM encoded private key file.")
    insecurePtr := flag.String("insecure", "true", "skip verifying cert chain")
    caCrtPtr    := flag.String("caCrt", "", "ca cert")

    flag.Parse()
    log.SetFlags(log.Lshortfile)
    fmt.Printf("sending to %s\n", *ipPtr + ":" + *portPtr)

    // Load client cert
    cert, err := tls.LoadX509KeyPair(*crtPtr, *keyPtr)
    if err != nil {
        log.Fatal(err)
    }

    insecure := false
    if *insecurePtr == "true" {
        fmt.Printf("Insecure TLS\n")
        insecure = true
    }

    var conf *tls.Config
    if *caCrtPtr != "" {
        caCert, err := ioutil.ReadFile(*caCrtPtr)
        if err != nil {
            log.Fatal(err)
        }
        caCertPool := x509.NewCertPool()
        caCertPool.AppendCertsFromPEM(caCert)

        conf = &tls.Config{
            Certificates: []tls.Certificate{cert},
            RootCAs:      caCertPool,
            InsecureSkipVerify: insecure,
        }
    } else {
        conf = &tls.Config{
            Certificates: []tls.Certificate{cert},
            InsecureSkipVerify: insecure,
        }
    }

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
