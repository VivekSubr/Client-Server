CURL_LIB=/home/vivek/test_curl/curl/lib/.libs
CURL_INC=/home/vivek/test_curl/curl/include
CLIENT_INC=../../http2/libcurl_client
OPEN_TELEMTRY_INC=-I/home/vivek/openTelemetry/opentelemetry-cpp/exporters/jaeger/include \
				  -I/home/vivek/openTelemetry/opentelemetry-cpp/api/include \
				  -I/home/vivek/openTelemetry/opentelemetry-cpp/ext/include \
				  -I/home/vivek/openTelemetry/opentelemetry-cpp/sdk/include

INC=-I$(CURL_INC) -I$(CLIENT_INC) $(OPEN_TELEMTRY_INC)

build:
	g++ -o example.exe $(INC) -L$(CURL_LIB) tracer.cc example.cc -lcurl -pthread 

clean: 
	rm -fr *.exe *.o