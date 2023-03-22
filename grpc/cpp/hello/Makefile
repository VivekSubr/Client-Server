ROOT_DIR=$(shell readlink -e ../../.)

proto:
	protoc --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` \
		--proto_path=$(ROOT_DIR)/proto/hello $(ROOT_DIR)/proto/hello/hello.proto

build: clean
	(mkdir -p build && cd build && cmake ..)

clean:
	rm -fr build *.pb*