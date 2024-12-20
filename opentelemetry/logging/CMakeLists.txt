cmake_minimum_required(VERSION 3.0.0)
project(LoggerExample VERSION 1.0.0)

include(FetchContent)
include(ExternalProject)
include(CMakePrintHelpers)

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

#install 'benchmark' as well, sudo apt install libbenchmark-dev
ExternalProject_Add(opentelemetry-cpp
    GIT_REPOSITORY https://github.com/open-telemetry/opentelemetry-cpp.git
    GIT_TAG main
    CONFIGURE_COMMAND cmake ${CMAKE_BINARY_DIR}/opentelemetry-cpp -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_CXX_STANDARD=20 -DBUILD_SHARED_LIBS=ON -DWITH_STL=ON -DWITH_OTLP_HTTP=ON -DBUILD_TESTING=OFF
    BUILD_COMMAND cmake --build ${CMAKE_BINARY_DIR}/opentelemetry-cpp --target all
    INSTALL_COMMAND cmake --install . --prefix ${CMAKE_BINARY_DIR}/opentelemetry-cpp
    PREFIX ${CMAKE_BINARY_DIR}
    BINARY_DIR opentelemetry-cpp
    SOURCE_DIR opentelemetry-cpp
)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIR})

add_library(Logger STATIC logger_factory.cc logger.cc fmt_logger.cc)

list(APPEND OTEL_LIB "-lopentelemetry_resources -lopentelemetry_common -lopentelemetry_trace -lopentelemetry_metrics")
list(APPEND OTEL_LIB "-lopentelemetry_proto -lopentelemetry_exporter_in_memory -lopentelemetry_exporter_ostream_span -lopentelemetry_logs")
list(APPEND OTEL_LIB "-lopentelemetry_exporter_ostream_logs -lopentelemetry_exporter_ostream_span -lopentelemetry_exporter_otlp_http")
list(APPEND OTEL_LIB "-lopentelemetry_otlp_recordable -lopentelemetry_exporter_otlp_http_log -lopentelemetry_exporter_otlp_http_client")

cmake_print_variables(OTEL_LIB)

target_link_directories(Logger PRIVATE ${CMAKE_BINARY_DIR}/opentelemetry-cpp/lib)
target_include_directories(Logger PRIVATE ${CMAKE_BINARY_DIR}/opentelemetry-cpp/include)
target_link_libraries(Logger PRIVATE ${OTEL_LIB} fmt::fmt)

# Setup testing
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

# Add test cpp file
add_executable(LoggerTest logger_test.cc)

include_directories(include)
target_link_directories(LoggerTest PRIVATE ${CMAKE_BINARY_DIR}/opentelemetry-cpp/lib)
target_include_directories(LoggerTest PRIVATE ${CMAKE_BINARY_DIR}/opentelemetry-cpp/include)

#Link test executable against gtest & gtest_main
target_link_libraries(LoggerTest  
        ${GTEST_LIBRARY} 
        ${GTEST_MAIN_LIBRARY}  
        ${OTEL_LIB}
        fmt::fmt)

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

