Opentelemetry implementations for tracing using jaeger, metrics using prometheus and logging using elastic.

Opentelemetry code is expected to be built in module/opentelemetry-cpp. 
* git submodule update --force --init --recursive to download dependencies

* build and install abseil

* GRPC is a dependency and hence must be built first,
    cd $ROOT/module/grpc
    mkdir -p cmake/build && cd cmake/build
    cmake -DCMAKE_PREFIX_PATH=$ROOT/abseil-cpp -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=/usr ../..
    (If this errors, may try recloning grpc repo)
    make -j 4
    sudo make install

* Next, opentelemetry needs to be built
    cd $ROOT/module/opentelemetry-cpp
    mkdir build && cd build
    cmake .. -DBUILD_SHARED_LIBS=ON -DWITH_ABSEIL=ON -DWITH_OTLP=ON -DWITH_OTLP_GRPC=ON -DWITH_JAEGER=ON -DBUILD_TESTING=OFF 