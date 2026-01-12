package main

import (
	"flag"
	"fmt"
	"net/http"

	"golang.org/x/net/http2"
	"golang.org/x/net/http2/h2c"
)

func main() {
	ipPtr := flag.String("ip", "0.0.0.0", "ip server listening to")
	portPtr := flag.String("port", "3000", "port server listening to")
	flag.Parse()

	h2s := &http2.Server{}
	handler := http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("x-dscp", "34")
		fmt.Print("Replied\n")
		fmt.Fprintf(w, "Hello, %v, http: %v\n", r.URL.Path, r.TLS == nil)
	})

	server := &http.Server{
		Addr:    *ipPtr + ":" + *portPtr,
		Handler: h2c.NewHandler(handler, h2s),
	}

	fmt.Printf("serving on %v:%v\n\n", *ipPtr, *portPtr)
	err := server.ListenAndServe()
	fmt.Printf("%v\n", err)
}
