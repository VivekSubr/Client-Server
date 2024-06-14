* Emscripten needs to be installed - https://emscripten.org/docs/getting_started/downloads.html#sdk-download-and-install
  In Ubuntu, sudo apt install emscripten
  
* Wasm filters use "Webassembly for proxies" project - https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/tree/main

* An alternative option is to build using envoy itself, eg: copy files to wasm-cc example, edit the BUILD file and do,
  bazel build --verbose_failures --sandbox_debug //examples/wasm-cc:envoy_filter_dscp.wasm