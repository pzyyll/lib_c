cmake_minimum_required(VERSION 3.7)
project(demo)

include_directories(C:\\cygwin64\\usr\\include)
include_directories(../)

aux_source_directory(../ LIBEV)
add_definitions(-DEV_STANDALONE)
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ${LIBEV})
add_executable(demo ${SOURCE_FILES})
target_link_libraries(demo libev.a)
target_link_libraries(demo -lpthread)