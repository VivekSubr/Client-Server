MODULE=../../module
NGHTTP2_LIB=$(MODULE)/nghttp2/install/lib
NGHTTP2_INC=$(MODULE)/nghttp2/install/include

build:
	g++ -g -I$(NGHTTP2_INC) -Wl,-rpath,$(NGHTTP2_LIB) -L$(NGHTTP2_LIB) main.cc -o client.exe -lnghttp2 

clean:
	rm -fr *.o *.exe