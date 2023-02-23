MODULE=../../../module
SRC=../
NGHTTP2_LIB=$(MODULE)/nghttp2/install/lib
NGHTTP2_INC=$(MODULE)/nghttp2/install/include
CURL_LIB=$(MODULE)/curl/lib
CURL_INC=$(MODULE)/curl/include

build:
	g++ -g -I$(SRC) -I$(NGHTTP2_INC) -L$(NGHTTP2_LIB) server_test.cc -o serverTest.exe \
	                                    -lnghttp2 -levent -lgtest -lgmock -lpthread

clean:
	rm -fr *.o *.exe