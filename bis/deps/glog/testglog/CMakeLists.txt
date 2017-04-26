cmake_minimum_required(VERSION 3.7)
project(testglog)

include_directories(../include)
link_directories(../lib)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(testglog ${SOURCE_FILES})

target_link_libraries(testglog libglog.a)
target_link_libraries(testglog -lpthread)