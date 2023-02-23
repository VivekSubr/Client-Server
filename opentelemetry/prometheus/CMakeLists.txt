cmake_minimum_required(VERSION 3.0.0)
project(MetricsExample VERSION 1.0.0)

find_package(CURL REQUIRED)
find_package(opentelemetry-cpp CONFIG REQUIRED)

add_library(Metrics STATIC metrics.cc)

target_include_directories(Metrics PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS})
target_link_libraries(Metrics PRIVATE ${OPENTELEMETRY_CPP_LIBRARIES})

# Setup testing
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

# Add test cpp file
add_executable(MetricsTest metrics_test.cc)

# Link test executable against gtest & gtest_main
target_link_libraries(MetricsTest ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})
target_include_directories(MetricsTest PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS})

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
