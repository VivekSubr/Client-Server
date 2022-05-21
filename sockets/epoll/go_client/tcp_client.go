package main

import (
	"flag"
    "net"
    "os"
)

func main() {
	ipPtr := flag.String("ip", "127.0.0.1", "ip client sends to")
	portPtr := flag.String("port", "3000", "port server listening to")
	flag.Parse()
	
    tcpAddr, err := net.ResolveTCPAddr("tcp", *ipPtr + ":" + *portPtr)
    if err != nil {
        println("ResolveTCPAddr failed:", err.Error())
        os.Exit(1)
    }

    conn, err := net.DialTCP("tcp", nil, tcpAddr)
    if err != nil {
        println("Dial failed:", err.Error())
        os.Exit(1)
    }

	sendStr := "client data"
    _, err = conn.Write([]byte(sendStr))
    if err != nil {
        println("Write to server failed:", err.Error())
        os.Exit(1)
    }

    println("write to server = ", sendStr)

    reply := make([]byte, 1024)

    _, err = conn.Read(reply)
    if err != nil {
        println("Write to server failed:", err.Error())
        os.Exit(1)
    }

    println("reply from server=", string(reply))

    conn.Close()
}