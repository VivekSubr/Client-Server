checkenv:
	if [test "$(CURL_LIB)" = ""] || [test "$(CURL_INCLUDE)" = ""] ; then \
        echo "ENV not set"; \
        exit 1; \
    fi

build: 
	#make -C go_server build
	make -C libmicrohttpd_server/http2.mk build
	g++ -o test.exe -L($CURL_LIB) -I($CURL_INCLUDE) http2_easy_client.cc -lcurl

clean:
	#make -C go_server clean
	make -f libmicrohttpd_server/http2.mk clean
	rm -rf http2_client *.o CMakeFiles cmake_install.cmake CMakeCache.txt *.log
