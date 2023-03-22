cmake_minimum_required(VERSION 3.1.2)
project(grpc_example CXX)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(Protobuf REQUIRED)
get_filename_component(PROTO_PATH ../../proto/hello REALPATH)
file(GLOB ProtoFiles "${PROTO_PATH}/*.proto")
message("found protofiles ${ProtoFiles}")

execute_process (
    COMMAND bash -c "which grpc_cpp_plugin"
    OUTPUT_VARIABLE grpc_plugin
)
exec_program(${PROTOBUF_PROTOC_EXECUTABLE} 
            ARGS --proto_path=${PROTO_PATH} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
            --plugin=protoc-gen-grpc=${grpc_plugin} ${ProtoFiles})

add_executable(client client.cc)
add_executable(server server.cc)

target_link_libraries(client protobuf::libprotobuf)
target_link_libraries(server protobuf::libprotobuf)
