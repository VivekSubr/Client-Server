build: build-tls
	make -C go_server build
	make -C go_client build

build-tls:
	make -C tls/server build
	make -C tls/client build

clean:
	make -C go_server  clean
	make -C go_client  clean
	make -C tls/server clean
	make -C tls/client clean
	rm -fr *log
