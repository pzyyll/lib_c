cmake_minimum_required(VERSION 3.7)

aux_source_directory(. BSPROTOSOURCES)
add_library(bsproto ${BSPROTOSOURCES})