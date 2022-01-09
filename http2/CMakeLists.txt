cmake_minimum_required(VERSION 3.0.0)
project(http2_client VERSION 1.0.0)

add_executable(http2_client http2_multi_client.cc)

target_include_directories(http2_client PRIVATE $ENV{CURL_INCLUDE})
target_link_directories(http2_client PRIVATE $ENV{CURL_LIB})

target_link_libraries(http2_client PRIVATE libcurl.so)
target_link_libraries(http2_client PRIVATE libpthread.so)
