PROXY_WASM_CPP_SDK=/sdk

include ${PROXY_WASM_CPP_SDK}/Makefile

build: 
	cd proxy-wasm-cpp-sdk/dscp
	docker run -v $PWD:/work -w /work wasmsdk:v3 /build_wasm.sh dscp-filter.wasm
	cd .. && cp proxy-wasm-cpp-sdk/dscp/dscp-filter.wasm .

init:
	git clone https://github.com/proxy-wasm/proxy-wasm-cpp-sdk.git
	docker build -t wasmsdk:v3 -f Dockerfile-sdk .
	mkdir -p proxy-wasm-cpp-sdk/dscp
	cp -f dscp-filter.cc Makefile proxy-wasm-cpp-sdk/dscp/.

clean:
	rm -fr *.wasm *.log proxy-wasm-cpp-sdk