package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"net/http"
	"time"
)

func main() {
	ipPtr := flag.String("ip", "localhost", "ip server listening to")
	portPtr := flag.String("port", "3000", "port server listening to")
	flag.Parse()

	url := "http://" + *ipPtr + ":" + *portPtr + "/example"
	postBody, _ := json.Marshal(map[string]string{
		"name":  "Subru",
		"email": "Subru@example.com",
	})
	responseBody := bytes.NewBuffer(postBody)

	fmt.Printf("sending to %q\n\n", url)
	for {
		rsp, err := http.Post(url, "application/json", responseBody)
		fmt.Printf("resp %v \n, err %v \n\n", rsp, err)
		time.Sleep(2 * time.Second)
	}
}
