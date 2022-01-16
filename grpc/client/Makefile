#this include path will be where the grpc plugin is installed
INC := /home/vivek/.local/include
LIB := /home/vivek/.local/lib
ABSL_LIB := -labsl_base -labsl_synchronization -labsl_raw_logging_internal

build:
	g++ -I$(INC) -L$(LIB) -g -o client.exe hello.pb.cc hello.grpc.pb.cc main.cc \
	                                                                 $(ABSL_LIB)  -lprotobuf -lgrpc++

clean:
	rm -f client.exe
