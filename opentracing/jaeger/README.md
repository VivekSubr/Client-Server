Opentelemetry CPP SDK needs to be built first,
* git submodule init
* cd $ROOT/modules/opentelmetry-cpp
* Follow instructions,
  mkdir build && cd build
  cmake .. -DBUILD_SHARED_LIBS=ON -DWITH_JAEGER=ON
  cmake --build . --target all
  ctest
  cmake --install . --config Debug --prefix ../.

Do these steps:
* ./init_jaeger.sh to start up jaeger
* ./cmake_build.sh to build the example app
* make and start go_server
* make and ./example in build folder, type 'udp' or 'http' to send a message
* Look at traces in http://localhost:16686
