Opentelemetry implementations for tracing using jaeger, metrics using prometheus and logging using elastic.

Opentelemetry code is expected to be built in module/opentelemetry-cpp. 
* git submodule update --force --init --recursive to download dependencies

* install abseil (eg: sudo apt install libabsl-dev)

* Try to use ci scripts of opentelemetery-cpp repo to install dependedencies if possible, 
    sudo -E ./ci/setup_ci_environment.sh
    sudo -E ./ci/install_protobuf.sh 
    sudo ./ci/setup_grpc.sh

* if not, install protobuf and then grpc,
    cd $ROOT/module/grpc
    mkdir -p cmake/build && cd cmake/build
    cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=/usr ../..
    (If this errors, may try recloning grpc repo)
    make -j 4
    sudo make install

* Next, opentelemetry needs to be built
    cd $ROOT/module/opentelemetry-cpp
    mkdir build && cd build
    cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_CXX_STANDARD=20 -DBUILD_SHARED_LIBS=ON -DWITH_ABSEIL=ON -DWITH_STL=ON -DWITH_OTLP_GRPC=ON -DBUILD_TESTING=OFF -DWITH_PROMETHEUS=ON -DCMAKE_INSTALL_PREFIX=$ROOT/module/opentelemetry-cpp
    cmake --build . --target all
