cmake_minimum_required(VERSION 3.4.1)

project(MapleEngine)

get_filename_component(EDITOR_LIB_SRC_DIR
                       ${CMAKE_CURRENT_LIST_DIR}/../lib
                       ABSOLUTE)


get_filename_component(ASSET_DIR
					  ${CMAKE_SOURCE_DIR}/../../Resources
                      ABSOLUTE)					   
					   

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")


file(GLOB SHADERS_GLSL 
	"${ASSET_DIR}/shaders/*.vert" 
	"${ASSET_DIR}/shaders/*.frag" 
	"${ASSET_DIR}/shaders/*.bat" 
)


file(GLOB VK_APP_SRC
	src/*.cpp
	src/*.h	
	src/Engine/*.h
	src/Engine/*.cpp
	src/Engine/Interface/*.h
	src/Engine/Interface/*.cpp
	src/Engine/Vulkan/*.h
	src/Engine/Vulkan/*.cpp
	src/Engine/Renderer/*.cpp
	src/Engine/Renderer/*.h
	src/Event/*.h
	src/Event/*.cpp
	src/Others/*.h
	src/Others/*.cpp
	src/Window/*.h
	src/Window/*.cpp
	src/Devices/*.h
	src/Devices/*.cpp
	src/ImGui/*.cpp
	src/ImGui/*.h
	src/FileSystem/*.cpp
	src/FileSystem/*.h
	src/Terrain/*.cpp
	src/Terrain/*.h
	src/Scene/*.cpp
	src/Scene/*.h
	src/Scripts/*.cpp
	src/Scripts/*.h
	src/Scene/Component/*.cpp
	src/Scene/Component/*.h
	src/Scene/Entity/*.cpp
	src/Scene/Entity/*.h
	src/Scene/System/*.h
	src/Scene/System/*.cpp
	src/Resources/*.cpp
	src/Resources/*.h
	src/Math/*.cpp
	src/Math/*.h
)


if (${Target} MATCHES "Windows")

add_compile_options("/std:c++17")
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_RAND_S -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES -DPLATFORM_WINDOWS -DPLATFORM_DESKTOP
	-DGLM_FORCE_DEPTH_ZERO_TO_ONE 
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "MapleEngine")

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${VK_APP_SRC})

source_group("Shaders\\GLSL" FILES ${SHADERS_GLSL})

add_library(MapleEngine ${VK_APP_SRC} ${KTX_SOURCES} ${SHADERS_GLSL})

target_include_directories(MapleEngine PUBLIC
	src
	${EDITOR_LIB_SRC_DIR}/opengl/include
	${EDITOR_LIB_SRC_DIR}/imgui/src
	${EDITOR_LIB_SRC_DIR}/spdlog/include
	${EDITOR_LIB_SRC_DIR}/stb_image
	${EDITOR_LIB_SRC_DIR}/vulkan/include
	${EDITOR_LIB_SRC_DIR}/tinyobjloader
	${EDITOR_LIB_SRC_DIR}/glm
	${EDITOR_LIB_SRC_DIR}/entt
	${EDITOR_LIB_SRC_DIR}/SPIRV-Cross
	${EDITOR_LIB_SRC_DIR}/ktx/include
	${EDITOR_LIB_SRC_DIR}/ktx/other_include
	${EDITOR_LIB_SRC_DIR}/cereal/include
)

target_link_libraries(MapleEngine 
	${EDITOR_LIB_SRC_DIR}/opengl/lib/${Arch}/glfw3.lib
	${EDITOR_LIB_SRC_DIR}/opengl/lib/${Arch}/opengl32.lib
	${EDITOR_LIB_SRC_DIR}/vulkan/lib/${Arch}/vulkan-1.lib
	${EDITOR_LIB_SRC_DIR}/vulkan/lib/${Arch}/VkLayer_utils.lib
	tinyobjloader
	imgui
	spirvCross
	ktx
)

endif()


