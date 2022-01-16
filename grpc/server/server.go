package main

import (
	"log"
	"net"
	pb "server/proto"

	"google.golang.org/grpc"
)

func main() {
	lis, err := net.Listen("tcp", ":4333")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	var opts []grpc.ServerOption
	grpcServer := grpc.NewServer(opts...)

	var exampleServer pb.ExampleServer
	pb.RegisterExampleServer(grpcServer, exampleServer)
	grpcServer.Serve(lis)
}
