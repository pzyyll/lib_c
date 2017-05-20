cmake_minimum_required(VERSION 3.7)
project(client)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../deps/protobuf-master/src/)

aux_source_directory(./ SOURCES)

add_subdirectory(./proto)

set(SOURCE_FILES ${SOURCES})

add_executable(client ${SOURCE_FILES})

target_link_libraries(client bsproto)
target_link_libraries(client -lprotobuf)
target_link_libraries(client -lpthread)