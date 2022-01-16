LIB=/home/vivek/test_curl/curl/lib/.libs
INC=/home/vivek/test_curl/curl/include

build:
	g++ -I$(INC) -L$(LIB) -o client.exe http2_multi_client.cc -lcurl -pthread

clean:
	rm -f *.o *.exe