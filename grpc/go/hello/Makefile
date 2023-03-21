ROOT_DIR=$(shell readlink -e ../../.)

proto:
	protoc --go_out=. --go-grpc_out=. --proto_path=$(ROOT_DIR)/proto/hello $(ROOT_DIR)/proto/hello/hello.proto

client:
	make -f client/Makefile build

server:
	make -f server/Makefile build

clean:
	rm -fr google.golang.org
	make -f client/Makefile clean
	make -f server/Makefile clean