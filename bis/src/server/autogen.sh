#!/bin/sh

BUILD_PATH="cmake_build_debug"
cp CMakeList.txt.linux CMakeList.txt
mkdir $BUILD_PATH
cd ./proto && ./autogen.sh
cd - && cd ./snslib && ./autogen.sh && make
cd - && cd $BUILD_PATH && cmake .. && make -j2
#make

