cmake_minimum_required(VERSION 3.7)

aux_source_directory(. PROTOSOURCES)
add_library(sinc ${PROTOSOURCES})