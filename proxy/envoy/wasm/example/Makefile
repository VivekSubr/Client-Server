#go mod edit -require=github.com/tetratelabs/proxy-wasm-go-sdk@main
#go mod download github.com/tetratelabs/proxy-wasm-go-sdk

build:
	tinygo build -o example-filter.wasm -scheduler=none -target=wasi ./example-filter.go

clean:
	go clean -testcache
	rm -fr *.wasm *.log
	
test: build
	go test -v -tags=proxytest ./...
