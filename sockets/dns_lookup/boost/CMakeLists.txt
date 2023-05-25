cmake_minimum_required(VERSION 3.0.0)
project(dns_boost VERSION 1.0.0)
set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")
SET(CMAKE_VERBOSE_MAKEFILE true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost 1.71 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories("../.")
add_executable(dns.exe dns.cc)
add_executable(dns_timer.exe dns_poll_timer.cc)
add_executable(dns_coroutine.exe dns_coroutine.cc)