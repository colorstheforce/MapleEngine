cmake_minimum_required(VERSION 3.4.1)

project(MapleLibrary VERSION 0.1.0 LANGUAGES CSharp)
INCLUDE(CSharpUtilities)
get_filename_component(LIB_SRC_DIR
                       ${CMAKE_CURRENT_LIST_DIR}
                       ABSOLUTE)

message( ${CMAKE_CURRENT_LIST_DIR})

get_filename_component(ASSET_DIR
					  ${CMAKE_SOURCE_DIR}/Assets
                      ABSOLUTE)		

file(GLOB GAME_APP_SRC
	${LIB_SRC_DIR}/*.cs
	${LIB_SRC_DIR}/Editor/*.cs
	${LIB_SRC_DIR}/ImGui/*.cs
	${LIB_SRC_DIR}/ImGui/Generated/*.cs
)

if (${Target} MATCHES "Windows")

#set(LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/Scripts)

SET(CMAKE_CSharp_FLAGS "/platform:AnyCPU /unsafe")



set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "MapleLibrary")

source_group(TREE ${LIB_SRC_DIR} FILES ${GAME_APP_SRC})

add_library(MapleLibrary SHARED ${GAME_APP_SRC})
set_property(TARGET MapleLibrary PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1")
set_property(
	TARGET 
	MapleLibrary 
	PROPERTY 
	VS_DOTNET_REFERENCES
	"System;System.Core;System.Numerics;System.Runtime;System.Runtime.CompilerServices.Unsafe"
)

SET(LIBRARY_OUTPUT_PATH ${ASSET_DIR})
endif()


