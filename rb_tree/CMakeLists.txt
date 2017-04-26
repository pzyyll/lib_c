cmake_minimum_required(VERSION 3.7)
project(rb_tree)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp src/rb_tree.cpp src/rb_tree.h)
add_executable(rb_tree ${SOURCE_FILES})