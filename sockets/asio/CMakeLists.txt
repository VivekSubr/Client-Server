cmake_minimum_required(VERSION 3.0.0)
project(AsioExample VERSION 1.0.0)
SET(CMAKE_CXX_COMPILER  "/usr/bin/clang++-15")
SET(CMAKE_VERBOSE_MAKEFILE true)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines-ts")

find_package(Boost 1.71 REQUIRED)

add_executable(asio_client client.cc)
add_executable(asio_server server.cc)

target_link_libraries(asio_client ${Boost_LIBRARIES})
target_link_libraries(asio_server ${Boost_LIBRARIES})

target_include_directories(asio_client PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(asio_server PRIVATE ${Boost_INCLUDE_DIRS})