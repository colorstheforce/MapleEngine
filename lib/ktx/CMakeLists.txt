cmake_minimum_required(VERSION 3.10)

project(ktx)


set(KTX_SOURCES
    lib/texture.c
    lib/hashlist.c
    lib/checkheader.c
    lib/swap.c
    lib/memstream.c
    lib/filestream.c)
	
	
add_library(ktx ${KTX_SOURCES})



target_include_directories(ktx PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>")

target_include_directories(ktx PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/other_include>")

