set -x
OPENTELEMETRY_PATH=`realpath ../../module/opentelemetry-cpp`
LIB_PATH="/usr/lib/x86_64-linux-gnu"
export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:$OPENTELEMETRY_PATH/build/cmake/opentelemetry-cpp/:$LIB_PATH

rm -fr build
mkdir -p build && cd build
cmake ..
