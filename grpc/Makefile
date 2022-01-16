build: proto
	make -C server/ build
	make -C server/ build

proto:
	#so, grpc is a plugin to protoc, hence the golang and cpp plugins need to installed and available on $PATH
	protoc hello.proto --go_out=server/. --go-grpc_out=server/.
	protoc hello.proto --cpp_out=client/. --grpc_out=client/. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` 

clean:
	make -C server/ clean
	make -C client/ clean
