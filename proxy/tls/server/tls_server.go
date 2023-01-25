package main

import (
    "fmt"
    "log"
    "flag"
    "crypto/tls"
    "net"
    "bufio"
)

func main() {
    ipPtr := flag.String("ip", "0.0.0.0", "ip server listening to")
    portPtr := flag.String("port", "3000", "port server listening to")
    crtPtr := flag.String("crt", "publickey.cer", "X.509 certificate")
    keyPtr := flag.String("key", "private.pem", "private key")
    flag.Parse()

    fmt.Printf("listening on %s\n", *ipPtr + ":" + *portPtr)
    log.SetFlags(log.Lshortfile)

    cer, err := tls.LoadX509KeyPair(*crtPtr, *keyPtr)
    if err != nil {
        log.Println(err)
        return
    }

    config := &tls.Config{
        Certificates: []tls.Certificate{cer},
        MinVersion:    tls.VersionTLS13,
    }
    
    ln, err := tls.Listen("tcp",  *ipPtr + ":" + *portPtr, config) 
    if err != nil {
        log.Println(err)
        return
    }
    defer ln.Close()

    for {
        conn, err := ln.Accept()
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
            log.Println(err)
            return
        }

        println(msg)

        n, err := conn.Write([]byte("world\n"))
        if err != nil {
            log.Println(n, err)
            return
        }

        fmt.Print("Replied\n")
    }
}
