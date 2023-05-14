cmake_minimum_required(VERSION 3.0.0)
project(AsioExample VERSION 1.0.0)
set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")
SET(CMAKE_VERBOSE_MAKEFILE true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost 1.71 REQUIRED)
include_directories("../.")

add_executable(asio_client client.cc)
add_executable(asio_server server.cc)

target_link_libraries(asio_client ${Boost_LIBRARIES})
target_link_libraries(asio_server ${Boost_LIBRARIES})

target_include_directories(asio_client PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(asio_server PRIVATE ${Boost_INCLUDE_DIRS})