cmake_minimum_required(VERSION 3.0.0)
project(TracerExample VERSION 1.0.0)

find_package(CURL REQUIRED) 
find_package(Thrift REQUIRED) 
find_package(opentelemetry-cpp CONFIG REQUIRED)
find_package(Boost 1.71 COMPONENTS log REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} )

include_directories(${CURL_INCLUDE_DIR})

add_library(Tracer STATIC tracer.cc http.cc logger/logger.cc)
add_executable(example example.cc)

target_include_directories(Tracer PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS})
target_link_libraries(Tracer PRIVATE ${OPENTELEMETRY_CPP_LIBRARIES})
target_link_libraries(Tracer PRIVATE ${Boost_LIBRARIES})
target_link_libraries(example PRIVATE Tracer ${CURL_LIBRARIES})
target_include_directories(example PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS})

# Setup testing
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

# Add test cpp file
add_executable(TracerTest tracer_test.cc)
add_executable(LoggerTest logger/logger_test.cc)

# Link test executable against gtest & gtest_main
target_link_libraries(TracerTest ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} ${Boost_LIBRARIES})
target_link_libraries(LoggerTest  ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} ${Boost_LIBRARIES})

target_include_directories(TracerTest PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_include_directories(LoggerTest PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_test(
    NAME TracerTest
    COMMAND TracerTest
)

add_test(
    NAME LoggerTest
    COMMAND LoggerTest
)

target_link_libraries(
    TracerTest
    gtest_main
    gtest
    gmock
    boost_log
    pthread
    Tracer
)

target_link_libraries(
    LoggerTest
    gtest_main
    gtest
    gmock
    boost_log
    pthread
)

message(STATUS "CURL_INCLUDE_DIR=${CURL_INCLUDE_DIR}")
message(STATUS "CURL_LIBRARIES=${CURL_LIBRARIES}")
message(STATUS "OPENTELEMETRY_CPP_INCLUDE_DIRS=${OPENTELEMETRY_CPP_INCLUDE_DIRS}")
message(STATUS "OPENTELEMETRY_CPP_LIBRARIES=${OPENTELEMETRY_CPP_LIBRARIES}")
message(STATUS "GTEST_INCLUDE_DIR=${GTEST_INCLUDE_DIR}")