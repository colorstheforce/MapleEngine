cmake_minimum_required(VERSION 3.10)

project(tellenc)

set(_SOURCES
    tellenc.cpp
  )

add_library(tellenc STATIC ${_SOURCES})

target_include_directories(tellenc PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>")


