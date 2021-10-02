checkenv:
	if [test "$(CURL_LIB)" = ""] || [test "$(CURL_INCLUDE)" = ""] ; then \
        echo "ENV not set"; \
        exit 1; \
    fi

build: 
	make -C server build
	g++ -o test.exe -L($CURL_LIB) -I($CURL_INCLUDE) http2_client.cc -lcurl

clean:
	make -C server clean
	rm -f test.exe *.o