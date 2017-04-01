cmake_minimum_required(VERSION 3.7)
project(mmap)

include_directories(C:\\cygwin64\\usr\\include)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(mmap ${SOURCE_FILES})