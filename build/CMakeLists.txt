cmake_minimum_required (VERSION 2.8)

add_definitions("-Wall -std=c++11")

project (imgUtil)

aux_source_directory(. DIR_SRCS)

add_library (imgUtil ${DIR_SRCS})

target_link_libraries(imgUtil libjpeg.a)
