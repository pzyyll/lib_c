cmake_minimum_required(VERSION 3.7)
project(binary_tree)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp binary_tree.cpp binary_tree.h)
add_executable(binary_tree ${SOURCE_FILES})