cmake_minimum_required(VERSION 3.4.1)

project(SampleApp)

get_filename_component(LIB_SRC_DIR
                       ${CMAKE_SOURCE_DIR}/lib
                       ABSOLUTE)

get_filename_component(APP_SRC_DIR
                       ${CMAKE_SOURCE_DIR}/app/src
                       ABSOLUTE)
					  

get_filename_component(APP_SAHDERS_DIR
                       ${CMAKE_SOURCE_DIR}/app/shaders
                       ABSOLUTE)

get_filename_component(ASSET_DIR
					  ${CMAKE_SOURCE_DIR}/Resources
                      ABSOLUTE)					   
					   


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")


add_subdirectory(lib/spdlog)
add_subdirectory(lib/imgui)
add_subdirectory(lib/stb_image)
add_subdirectory(lib/tinyobjloader)
add_subdirectory(lib/SPIRV-Cross)
add_subdirectory(lib/ktx)


file(GLOB VK_APP_SRC
	${APP_SRC_DIR}/*.cpp
	${APP_SRC_DIR}/*.h	
	${APP_SRC_DIR}/Engine/*.h
	${APP_SRC_DIR}/Engine/*.cpp
	${APP_SRC_DIR}/Engine/Interface/*.h
	${APP_SRC_DIR}/Engine/Interface/*.cpp
	${APP_SRC_DIR}/Engine/Vulkan/*.h
	${APP_SRC_DIR}/Engine/Vulkan/*.cpp
	${APP_SRC_DIR}/Engine/Renderer/*.cpp
	${APP_SRC_DIR}/Engine/Renderer/*.h
	${APP_SRC_DIR}/Event/*.h
	${APP_SRC_DIR}/Event/*.cpp
	${APP_SRC_DIR}/Others/*.h
	${APP_SRC_DIR}/Others/*.cpp
	${APP_SRC_DIR}/Editor/*.h
	${APP_SRC_DIR}/Editor/*.cpp
	${APP_SRC_DIR}/Window/*.h
	${APP_SRC_DIR}/Window/*.cpp
	${APP_SRC_DIR}/Devices/*.h
	${APP_SRC_DIR}/Devices/*.cpp
	${APP_SRC_DIR}/ImGui/*.cpp
	${APP_SRC_DIR}/ImGui/*.h
	${APP_SRC_DIR}/FileSystem/*.cpp
	${APP_SRC_DIR}/FileSystem/*.h
	${APP_SRC_DIR}/Terrain/*.cpp
	${APP_SRC_DIR}/Terrain/*.h
	${APP_SRC_DIR}/Scene/*.cpp
	${APP_SRC_DIR}/Scene/*.h
	${APP_SRC_DIR}/Scripts/*.cpp
	${APP_SRC_DIR}/Scripts/*.h
	${APP_SRC_DIR}/Scene/Component/*.cpp
	${APP_SRC_DIR}/Scene/Component/*.h
	${APP_SRC_DIR}/Scene/Entity/*.cpp
	${APP_SRC_DIR}/Scene/Entity/*.h
	${APP_SRC_DIR}/Scene/System/*.h
	${APP_SRC_DIR}/Scene/System/*.cpp
	${APP_SRC_DIR}/Resources/*.cpp
	${APP_SRC_DIR}/Resources/*.h
	${APP_SRC_DIR}/Math/*.cpp
	${APP_SRC_DIR}/Math/*.h
)


if (${Target} MATCHES "Windows")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_compile_options("/std:c++17")
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_RAND_S -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES -DPLATFORM_WINDOWS -DPLATFORM_DESKTOP
	-DGLM_FORCE_DEPTH_ZERO_TO_ONE 
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "SampleApp")

source_group(TREE ${APP_SRC_DIR} FILES ${VK_APP_SRC})

add_executable(SampleApp ${VK_APP_SRC} ${KTX_SOURCES})

set_property(TARGET SampleApp PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${ASSET_DIR})

set_target_properties(SampleApp PROPERTIES COMPILE_FLAGS "/MP /wd4819 /arch:SSE /std:c++17")

string(REPLACE "/" "\\" GLEW32_PATH ${LIB_SRC_DIR}/opengl/lib/${Arch}/glew32.dll)

string(REPLACE "/" "\\" GLEW32_OUT_PATH ${ASSET_DIR}/)

add_custom_command(TARGET SampleApp 
				   POST_BUILD
	COMMAND powershell Copy-Item ${GLEW32_PATH} ${GLEW32_OUT_PATH}
)

target_include_directories(SampleApp PUBLIC
	${APP_SRC_DIR}
	lib/opengl/include
	lib/imgui/src
	lib/spdlog/include
	lib/stb_image
	lib/vulkan/include
	lib/tinyobjloader
	lib/glm
	lib/entt
	lib/SPIRV-Cross
	lib/ktx/include
	lib/ktx/other_include
	lib/cereal/include
)

target_link_libraries(SampleApp 
	${CMAKE_SOURCE_DIR}/lib/opengl/lib/${Arch}/glfw3.lib
	${CMAKE_SOURCE_DIR}/lib/opengl/lib/${Arch}/opengl32.lib
	${CMAKE_SOURCE_DIR}/lib/vulkan/lib/${Arch}/vulkan-1.lib
	${CMAKE_SOURCE_DIR}/lib/vulkan/lib/${Arch}/VkLayer_utils.lib
	tinyobjloader
	imgui
	spirvCross
	ktx
)

endif()


