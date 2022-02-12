CURL_LIB=/home/vivek/libs/curl-7.81.0/lib/.libs
CURL_INC=/home/vivek/libs/curl-7.81.0/include
CLIENT_INC=../../http2/libcurl_client
OPEN_TELEMTRY_INC=-I/home/vivek/libs/opentelemetry-cpp/exporters/jaeger/include \
				  -I/home/vivek/libs/opentelemetry-cpp/api/include \
				  -I/home/vivek/libs/opentelemetry-cpp/ext/include \
				  -I/home/vivek/libs/opentelemetry-cpp/sdk/include
OPEN_TELEMTRY_LIB=-L/home/vivek/libs/opentelemetry-cpp/bazel-bin/exporters/jaeger \
				  -L/home/vivek/libs/opentelemetry-cpp/bazel-bin/exporters/jaeger/thrift/lib \
				  -L/home/vivek/libs/opentelemetry-cpp/bazel-bin/sdk/src/trace

INC=-I$(CURL_INC) -I$(CLIENT_INC) $(OPEN_TELEMTRY_INC)

build:
	g++ -o example.exe $(INC) -L$(CURL_LIB) $(OPEN_TELEMTRY_LIB) tracer.cc example.cc -lcurl -pthread \
					-lthrift -lthriftnb -lthriftz -ltrace -lopentelemetry_exporter_jaeger_trace

clean: 
	rm -fr *.exe *.o