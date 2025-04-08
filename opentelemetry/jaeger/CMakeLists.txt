cmake_minimum_required(VERSION 3.16) 
project(TracerExample VERSION 1.0.0) 
include(ExternalProject) 
include(FetchContent)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE) 

#installed abseil as absl in this dir
#set(absl_DIR "/home/vivek/Client-Server/module/absl/lib/cmake/absl")
find_package(absl REQUIRED)
find_package(Protobuf REQUIRED) #capital P!
find_package(CURL REQUIRED) 

ExternalProject_Add(
    opentelemetry-cpp 
    GIT_REPOSITORY https://github.com/open-telemetry/opentelemetry-cpp.git 
    GIT_TAG v1.18.0
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/opentelemetry_install 
        -DCMAKE_VERBOSE_MAKEFILE=ON 
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON 
        -DCMAKE_CXX_STANDARD=17 
        -DBUILD_SHARED_LIBS=ON 
        -DWITH_ABSEIL=OFF 
        -DWITH_STL=ON 
        -DWITH_OTLP_HTTP=ON 
        -DWITH_OTLP_GRPC=ON 
        -DBUILD_TESTING=OFF 
        -DWITH_PROMETHEUS=OFF 
)

include_directories(${CURL_INCLUDE_DIR})
add_library(Tracer STATIC tracer.cc http.cc) 
add_executable(example example.cc)

set(LIB_PATH ${CMAKE_BINARY_DIR}/opentelemetry_install/lib)
list(APPEND OTEL_LIB "libopentelemetry_resources.a libopentelemetry_common.a libopentelemetry_trace.a libopentelemetry_metrics.a")
list(APPEND OTEL_LIB "libopentelemetry_exporter_in_memory.a libopentelemetry_exporter_ostream_span.a")
list(APPEND OTEL_LIB "libopentelemetry_version.a libopentelemetry_exporter_otlp_grpc.a libopentelemetry_exporter_otlp_http.a")

message("Linking otel ${OTEL_LIB}")
message("OPENTELEMETRY_CPP_INCLUDE_DIRS=${OPENTELEMETRY_CPP_INCLUDE_DIRS}")
message("OPENTELEMETRY_CPP_LIBRARIES=${OPENTELEMETRY_CPP_LIBRARIES}")
target_include_directories(Tracer PUBLIC  ${CMAKE_BINARY_DIR}/opentelemetry_install/include) 
target_link_libraries     (Tracer PUBLIC) 

target_include_directories(example PRIVATE ${CMAKE_BINARY_DIR}/opentelemetry_install/include ${CURL_INCLUDE_DIR}) 
target_link_directories   (example PRIVATE ${CMAKE_BINARY_DIR}/opentelemetry_install/lib)  
target_link_libraries(example PRIVATE 
        Tracer #Note that in linking order, this comes first
        opentelemetry_resources opentelemetry_common opentelemetry_trace opentelemetry_metrics
        opentelemetry_exporter_in_memory opentelemetry_exporter_ostream_span
        opentelemetry_version opentelemetry_exporter_otlp_grpc opentelemetry_exporter_otlp_http
        CURL::libcurl) 

#Setup testing
enable_testing() 
include_directories(${GTEST_INCLUDE_DIR})

#Add test cpp file
add_executable(TracerTest tracer_test.cc)

#Link test executable against gtest & gtest_main
target_include_directories(TracerTest PRIVATE 
                            ${CMAKE_BINARY_DIR}/opentelemetry_install/include                                                              
                            ${GTEST_LIBRARY} 
                            ${GTEST_MAIN_LIBRARY} ) 
target_link_directories   (TracerTest PRIVATE ${CMAKE_BINARY_DIR}/opentelemetry_install/lib)  

target_link_libraries(TracerTest PRIVATE     
    gtest_main
    gtest
    gmock 
    pthread 
    Tracer
    opentelemetry_resources opentelemetry_common opentelemetry_trace opentelemetry_metrics
    opentelemetry_exporter_in_memory opentelemetry_exporter_ostream_span
    opentelemetry_version opentelemetry_exporter_otlp_grpc opentelemetry_exporter_otlp_http
)

add_dependencies(Tracer     opentelemetry-cpp) 
add_dependencies(TracerTest opentelemetry-cpp)
add_dependencies(example    opentelemetry-cpp) 

add_test(
    NAME TracerTest 
    COMMAND TracerTest )
