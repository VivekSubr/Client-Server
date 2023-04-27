cmake_minimum_required(VERSION 3.0.0)
project(MetricsExample VERSION 1.0.0)

find_package(protobuf REQUIRED)
find_package(CURL REQUIRED)
find_package(absl REQUIRED)
find_package(opentelemetry-cpp CONFIG REQUIRED)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")
find_package(Boost 1.74.0 COMPONENTS program_options) 

add_library(Metrics STATIC metrics_mock.cc metrics.cc)

target_include_directories(Metrics PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS})
target_link_libraries(Metrics PRIVATE ${OPENTELEMETRY_CPP_LIBRARIES})
target_compile_options(Metrics PRIVATE -Wall -Wextra -Wpedantic -O0 -g3 -ggdb)
target_compile_definitions(Metrics PRIVATE PROMETHEUS_EXPORTER)

add_executable(example example.cc)
target_link_libraries(example PRIVATE Metrics ${OPENTELEMETRY_CPP_LIBRARIES} ${Boost_LIBRARIES}) 
target_include_directories(example PRIVATE ${Boost_INCLUDE_DIRS})

# Setup testing
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

# Add test cpp file
add_executable(MetricsTest metrics_test.cc)

# Link test executable against gtest & gtest_main
target_link_libraries(MetricsTest ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})
target_link_libraries(MetricsTest ${OPENTELEMETRY_CPP_LIBRARIES}) #since .so, have to link here
target_include_directories(MetricsTest PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS})
target_compile_options(MetricsTest PRIVATE -Wall -Wextra -Wpedantic -O0 -g3 -ggdb)

add_test(
    NAME MetricsTest
    COMMAND MetricsTest
)

target_link_libraries(
    MetricsTest
    gtest_main
    gtest
    gmock
    pthread
    Metrics
)

message(STATUS "OPENTELEMETRY_CPP_INCLUDE_DIRS=${OPENTELEMETRY_CPP_INCLUDE_DIRS}")
