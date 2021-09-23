cmake_minimum_required(VERSION 3.4.1)

project(Maple-Engine)

get_filename_component(LIB_SRC_DIR
                       ${CMAKE_SOURCE_DIR}/Maple/lib
                       ABSOLUTE)

get_filename_component(APP_SRC_DIR  
                       ${CMAKE_SOURCE_DIR}/Editor/src ABSOLUTE)

get_filename_component(GAME_SRC_DIR
                       ${CMAKE_SOURCE_DIR}/Game/src ABSOLUTE)


get_filename_component(ASSET_DIR
					  ${CMAKE_SOURCE_DIR}/../Assets
                      ABSOLUTE)					   
					   
get_filename_component(TOOL_CHAIN_DIR
					  ${CMAKE_SOURCE_DIR}/../Toolchains
						ABSOLUTE)		


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")


add_subdirectory(Maple)
add_subdirectory(Scripts)


file(GLOB VK_APP_SRC
	${APP_SRC_DIR}/*.cpp
	${APP_SRC_DIR}/*.h	
	${APP_SRC_DIR}/Plugin/*.cpp
	${APP_SRC_DIR}/Plugin/*.h	
	${APP_SRC_DIR}/UI/*.cpp
	${APP_SRC_DIR}/UI/*.h	
)

file(GLOB GAME_APP_SRC
	${GAME_SRC_DIR}/*.cpp
	${GAME_SRC_DIR}/*.h	
)

if (${Target} MATCHES "Windows")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_compile_options("/std:c++17")
add_definitions(
	-D_CRT_SECURE_NO_WARNINGS 
	-D_CRT_RAND_S 
	-D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 
	-DPLATFORM_WINDOWS 
	-DPLATFORM_DESKTOP
	-DGLM_FORCE_DEPTH_ZERO_TO_ONE 
	-DMAPLE_DYNAMIC
	-DTOOLS_DIR=${TOOL_CHAIN_DIR}
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Maple")

source_group(TREE ${APP_SRC_DIR} FILES ${VK_APP_SRC})

source_group(TREE ${GAME_SRC_DIR} FILES ${GAME_APP_SRC})


add_executable(Editor ${VK_APP_SRC})

add_executable(Game ${GAME_APP_SRC})

set_property(TARGET Editor Game PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${ASSET_DIR})

set_target_properties(Editor PROPERTIES COMPILE_FLAGS "/MP /wd4819 /arch:SSE -DBuildEditor")

set_target_properties(Game PROPERTIES COMPILE_FLAGS "/MP /wd4819 /arch:SSE ")

string(REPLACE "/" "\\" GLEW32_PATH ${LIB_SRC_DIR}/opengl/lib/${Arch}/glew32.dll)

string(REPLACE "/" "\\" GLEW32_OUT_PATH ${ASSET_DIR}/)

add_custom_command(TARGET Editor Game
				   POST_BUILD
	COMMAND powershell Copy-Item ${GLEW32_PATH} ${GLEW32_OUT_PATH}
)


add_custom_command(TARGET Editor Game
				   POST_BUILD
	COMMAND powershell Copy-Item ${ASSET_DIR}/${CMAKE_BUILD_TYPE}/MapleLibrary.dll ${GLEW32_OUT_PATH}
)

add_custom_command(TARGET Editor Game
				   POST_BUILD
	COMMAND powershell Copy-Item ${ASSET_DIR}/${CMAKE_BUILD_TYPE}/MapleEngine.dll ${GLEW32_OUT_PATH}
)



target_include_directories(Editor PUBLIC
	${APP_SRC_DIR}
	${LIB_SRC_DIR}/opengl/include
	${LIB_SRC_DIR}/imgui/src
	${LIB_SRC_DIR}/spdlog/include
	${LIB_SRC_DIR}/stb_image
	${LIB_SRC_DIR}/vulkan/include
	${LIB_SRC_DIR}/tinyobjloader
	${LIB_SRC_DIR}/glm
	${LIB_SRC_DIR}/entt
	${LIB_SRC_DIR}/SPIRV-Cross
	${LIB_SRC_DIR}/ktx/include
	${LIB_SRC_DIR}/ktx/other_include
	${LIB_SRC_DIR}/cereal/include
)


target_include_directories(Game PUBLIC
	${GAME_APP_SRC}
	${LIB_SRC_DIR}/opengl/include
	${LIB_SRC_DIR}/imgui/src
	${LIB_SRC_DIR}/spdlog/include
	${LIB_SRC_DIR}/stb_image
	${LIB_SRC_DIR}/vulkan/include
	${LIB_SRC_DIR}/tinyobjloader
	${LIB_SRC_DIR}/glm
	${LIB_SRC_DIR}/entt
	${LIB_SRC_DIR}/SPIRV-Cross
	${LIB_SRC_DIR}/ktx/include
	${LIB_SRC_DIR}/ktx/other_include
	${LIB_SRC_DIR}/cereal/include
)

target_link_libraries(
	Editor 
	MapleEngine
)

target_link_libraries(
	Game 
	MapleEngine
)



endif()


