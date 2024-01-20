cmake_minimum_required(VERSION 3.0.0)
project(TracerExample VERSION 1.0.0)

include(FetchContent)

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

find_package(opentelemetry-cpp CONFIG REQUIRED)

include_directories( ${Boost_INCLUDE_DIRS} )
include_directories(${CURL_INCLUDE_DIR})

add_library(Logger STATIC logger_factory.cc logger.cc fmt_logger.cc)

target_include_directories(Logger PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS})
target_link_libraries(Logger PRIVATE ${OPENTELEMETRY_CPP_LIBRARIES} fmt::fmt)

# Setup testing
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

# Add test cpp file
add_executable(LoggerTest logger_test.cc)

# Link test executable against gtest & gtest_main
target_link_libraries(LoggerTest  
        ${GTEST_LIBRARY} 
        ${GTEST_MAIN_LIBRARY}  
        fmt::fmt)

include_directories(include)
target_include_directories(LoggerTest PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS})

add_test(
    NAME LoggerTest
    COMMAND LoggerTest
)

target_link_libraries(
    LoggerTest
    gtest_main
    gtest
    gmock
    pthread
    Logger
)

