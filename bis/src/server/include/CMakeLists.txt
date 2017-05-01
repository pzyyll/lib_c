cmake_minimum_required(VERSION 3.7)

aux_source_directory(. SOURCES)
add_library(sinc ${SOURCES})