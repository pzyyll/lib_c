cmake_minimum_required(VERSION 3.7)
project(hash_map)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp hash_table.h)
add_executable(hash_map ${SOURCE_FILES})