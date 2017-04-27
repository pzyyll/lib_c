cmake_minimum_required(VERSION 3.7)
project(demo)

include_directories(C:\\cygwin64\\usr\\include)
include_directories(../)

aux_source_directory(../ LIBEV)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ${LIBEV})
add_executable(demo ${SOURCE_FILES})