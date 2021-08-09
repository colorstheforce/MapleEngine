cmake_minimum_required(VERSION 3.4.1)

project(MapleLibrary VERSION 0.1.0 LANGUAGES CSharp)

get_filename_component(LIB_SRC_DIR
                       ${CMAKE_SOURCE_DIR}/Scripts
                       ABSOLUTE)


file(GLOB GAME_APP_SRC
	${LIB_SRC_DIR}/*.cs
)

if (${Target} MATCHES "Windows")

#set(LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/Scripts)
 
set(OUT_DIR ${CMAKE_SOURCE_DIR}/Scripts)
 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "MapleLibrary")

source_group(TREE ${LIB_SRC_DIR} FILES ${GAME_APP_SRC})

add_library(MapleLibrary SHARED ${GAME_APP_SRC})



endif()


