cmake_minimum_required(VERSION 3.13)
project("wren")

file(GLOB_RECURSE wren_sources ${CMAKE_CURRENT_SOURCE_DIR}/wren/src/vm/*.c ${CMAKE_CURRENT_SOURCE_DIR}/wren/src/vm/*.h ${CMAKE_CURRENT_SOURCE_DIR}/wren/src/vm/*.inc)
add_library(wren ${wren_sources})

target_include_directories(wren PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/wren/src/include)
target_compile_definitions(wren PUBLIC WREN_OPT_META=0)
target_compile_definitions(wren PUBLIC WREN_OPT_RANDOM=0)