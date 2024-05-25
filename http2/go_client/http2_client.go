package main

import (
	"crypto/tls"
	"context"
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"net"
	"net/http"
	"time"
	"io"

	"golang.org/x/net/http2"
)

func main() {
	ipPtr := flag.String("ip", "localhost", "ip server listening to")
	portPtr := flag.String("port", "3000", "port server listening to")
	flag.Parse()

	client := http.Client{
		Transport: &http2.Transport{
			AllowHTTP: true,// So http2.Transport doesn't complain the URL scheme isn't 'https'
			DialTLSContext: func(ctx context.Context, n, a string, _ *tls.Config) (net.Conn, error) {
				// Pretend we are dialing a TLS endpoint. Note, we ignore the passed tls.Config
				var d net.Dialer
				return d.DialContext(ctx, n, a)
			},
		},
	}

	url := "http://" + *ipPtr + ":" + *portPtr + "/example"
	postBody, _ := json.Marshal(map[string]string{
		"name":  "Subru",
		"email": "Subru@example.com",
	})

	req, _ := http.NewRequest("POST", url, bytes.NewBuffer(postBody))
    req.Header.Set("X-Custom-Header", "custom-header-test")
    req.Header.Set("Content-Type", "application/json")

	fmt.Printf("sending to %q\n\n", url)
	for {
		resp, err := client.Do(req)
		if err != nil {
			panic(err)
		}
		defer resp.Body.Close()
		
		fmt.Println("response Status:", resp.Status)
		fmt.Println("response Headers:", resp.Header)
		body, _ := io.ReadAll(resp.Body)
		fmt.Println("response Body:", string(body))

		time.Sleep(2 * time.Second)
	}
}
