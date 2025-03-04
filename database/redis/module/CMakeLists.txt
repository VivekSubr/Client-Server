cmake_minimum_required(VERSION 3.0.0)
project(RedisExampleModule VERSION 1.0.0)

include(ExternalProject)
ExternalProject_Add(redis
    GIT_REPOSITORY https://github.com/redis/redis.git
    GIT_TAG 7.2 #should be same as redis-server version
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make
    INSTALL_COMMAND ""
    PREFIX ${CMAKE_BINARY_DIR}
    BINARY_DIR redis
    SOURCE_DIR redis
)

add_library(exampleModule SHARED example.c)
target_include_directories(exampleModule PRIVATE ${CMAKE_BINARY_DIR}/redis)
