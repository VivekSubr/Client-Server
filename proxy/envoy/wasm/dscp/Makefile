MODULE_PATH=../../../../module
PROXY_WASM_CPP_SDK=${MODULE_PATH}/proxy-wasm-cpp-sdk
CPP_API:=${PROXY_WASM_CPP_SDK}
CPP_CONTEXT_LIB = ${CPP_API}/proxy_wasm_intrinsics.cc
ABSL = ${MODULE_PATH}/abseil-cpp/build
ABSL_LIBS = ${ABSL}/absl/strings/libabsl_strings.a ${ABSL}/absl/strings/libabsl_strings_internal.a  ${ABSL}/absl/strings/libabsl_str_format_internal.a

build: 
	em++ --no-entry -s EXPORTED_FUNCTIONS=['_malloc'] -s LLD_REPORT_UNDEFINED --std=c++17 -O3 -flto \
	-I${CPP_API} -I${CPP_API}/google/protobuf \
    -I/usr/local/include -I/usr/include -I/usr/include/x86_64-linux-gnu -I/usr/x86_64-linux-gnu/include -I${ABSL} \
	dscp-filter.cc \
	${CPP_API}/proxy_wasm_intrinsics.pb.cc ${CPP_CONTEXT_LIB} ${CPP_API}/libprotobuf.a ${ABSL_LIBS} -o dscp-filter.wasm

docker-init:
	mkdir -p work
	cp *.cc *.h work/.
	cd ${PROXY_WASM_CPP_SDK} && docker build -t wasmsdk:v2 -f Dockerfile-sdk .

docker: docker-init
	

clean:
	rm -fr work *.wasm *.log