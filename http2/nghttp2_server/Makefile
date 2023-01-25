MODULE=../../module
NGHTTP2_LIB=$(MODULE)/nghttp2/install/lib
NGHTTP2_INC=$(MODULE)/nghttp2/install/include

build:
	g++ -g -I$(NGHTTP2_INC) -L$(NGHTTP2_LIB) server.cc -o server.exe -lnghttp2 -levent

clean:
	rm -fr *.o *.exe