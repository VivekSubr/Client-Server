cmake_minimum_required(VERSION 3.0.0)
project(TracerExample VERSION 1.0.0)

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 
find_package(Thrift REQUIRED) 
find_package(opentelemetry-cpp CONFIG REQUIRED)

include_directories(${CURL_INCLUDE_DIR})

add_library(Tracer STATIC tracer.cc http.cc)
add_executable(example example.cc)

target_include_directories(Tracer PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS})
target_link_libraries(Tracer PRIVATE ${OPENTELEMETRY_CPP_LIBRARIES})
target_link_libraries(example PRIVATE Tracer)
target_include_directories(example PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS})
target_link_libraries(example PRIVATE ${CURL_LIBRARIES})

message(STATUS "CURL_INCLUDE_DIR=${CURL_INCLUDE_DIR}")
message(STATUS "CURL_LIBRARIES=${CURL_LIBRARIES}")
message(STATUS "OPENTELEMETRY_CPP_INCLUDE_DIRS=${OPENTELEMETRY_CPP_INCLUDE_DIRS}")
message(STATUS "OPENTELEMETRY_CPP_LIBRARIES=${OPENTELEMETRY_CPP_LIBRARIES}")