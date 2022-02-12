set -x
OPENTELEMETRY_PATH=`realpath ../../module/opentelemetry-cpp`
THRIFT_PATH=`realpath ../../module/thrift/build`
THRIFT_ADDL=`realpath ../../module/thrift/build/cmake`
export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:$THRIFT_PATH:$THRIFT_ADDL:$OPENTELEMETRY_PATH/build/cmake/opentelemetry-cpp/
                      
rm -fr build
mkdir -p build && cd build
cmake ..
