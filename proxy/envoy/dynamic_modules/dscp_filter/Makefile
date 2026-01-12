build: #Check envoy commit being used matches with https://github.com/envoyproxy/dynamic-modules-examples/blob/main/rust/Cargo.toml 
	cargo build --release
	mv target/release/libdscp_filter.so .

test:
	cargo test
	
clean:
	cargo clean
	rm -f *.log *.so

init:
	cargo fetch
