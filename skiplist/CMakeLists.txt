cmake_minimum_required(VERSION 3.7)
project(skiplist)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp src/skiplist.h src/skiplist.cpp)
add_executable(skiplist ${SOURCE_FILES})