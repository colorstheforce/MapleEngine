cmake_minimum_required(VERSION 3.4.1)

project(MapleEngine)

get_filename_component(ENGINE_LIB_SRC_DIR
                       ${CMAKE_CURRENT_LIST_DIR}/lib
                       ABSOLUTE)


get_filename_component(ENGINE_ASSET_DIR
					  ${CMAKE_CURRENT_LIST_DIR}/../../Assets
                      ABSOLUTE)					   
					   
get_filename_component(TOOL_CHAIN_DIR
					  ${CMAKE_CURRENT_LIST_DIR}/../../Toolchains
                      ABSOLUTE)					   
					   					  

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")



add_subdirectory(lib/spdlog)
add_subdirectory(lib/stb_image)
add_subdirectory(lib/tinyobjloader)
add_subdirectory(lib/SPIRV-Cross)
add_subdirectory(lib/ktx)
add_subdirectory(lib/charset-detect)
add_subdirectory(lib/zlib)
add_subdirectory(lib/lua)
add_subdirectory(lib/tracy)

file(GLOB SHADERS_GLSL 
	"${ENGINE_ASSET_DIR}/shaders/*.vert" 
	"${ENGINE_ASSET_DIR}/shaders/*.frag" 
	"${ENGINE_ASSET_DIR}/shaders/*.bat" 
)

file(GLOB IMGUI_SRC 
	${ENGINE_LIB_SRC_DIR}/imgui/src/imgui_tables.cpp
	${ENGINE_LIB_SRC_DIR}/imgui/src/imgui_draw.cpp
	${ENGINE_LIB_SRC_DIR}/imgui/src/imgui_widgets.cpp
	${ENGINE_LIB_SRC_DIR}/imgui/src/imgui_widgets_user.cpp
	${ENGINE_LIB_SRC_DIR}/imgui/src/imgui.cpp
	${ENGINE_LIB_SRC_DIR}/imgui/src/imgui_impl_opengl3.cpp
	${ENGINE_LIB_SRC_DIR}/imgui/src/imgui_impl_opengl3.cpp
	${ENGINE_LIB_SRC_DIR}/imgui/src/TextEditor.cpp
	${ENGINE_LIB_SRC_DIR}/imgui/src/imgui_impl_vulkan.cpp
	${ENGINE_LIB_SRC_DIR}/imgui/src/imgui_impl_glfw.cpp
	${ENGINE_LIB_SRC_DIR}/imgui/src/ImGuizmo.cpp
	${ENGINE_LIB_SRC_DIR}/imgui/src/cimgui.cpp
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
	src/Scripts/Mono/*.cpp
	src/Scripts/Mono/*.h
	src/Scripts/Lua/*.cpp
	src/Scripts/Lua/*.h
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
	src/Thread/*.cpp
	src/Thread/*.h
)


if (${Target} MATCHES "Windows")

	add_compile_options("/std:c++17")
	add_definitions(
		-D_CRT_SECURE_NO_WARNINGS 
		-D_CRT_RAND_S 
		-D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 
		-DPLATFORM_WINDOWS
		-DPLATFORM_DESKTOP
		-DGLM_FORCE_DEPTH_ZERO_TO_ONE
		-DMAPLE_DYNAMIC
		-DMAPLE_ENGINE
		-DMAPLE_PROFILE
	)

	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

	source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${VK_APP_SRC})

	source_group("Shaders\\GLSL" FILES ${SHADERS_GLSL})

	add_library(MapleEngine SHARED ${VK_APP_SRC} ${KTX_SOURCES} ${SHADERS_GLSL} ${IMGUI_SRC})

	SET_TARGET_PROPERTIES(MapleEngine PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ASSET_DIR})
	#set(LIBRARY_OUTPUT_PATH ${ASSET_DIR})
	target_include_directories(MapleEngine PUBLIC
		src
		${ENGINE_LIB_SRC_DIR}/opengl/include
		${ENGINE_LIB_SRC_DIR}/imgui/src
		${ENGINE_LIB_SRC_DIR}/spdlog/include
		${ENGINE_LIB_SRC_DIR}/stb_image
		${ENGINE_LIB_SRC_DIR}/vulkan/include
		${ENGINE_LIB_SRC_DIR}/tinyobjloader
		${ENGINE_LIB_SRC_DIR}/glm
		${ENGINE_LIB_SRC_DIR}/entt
		${ENGINE_LIB_SRC_DIR}/SPIRV-Cross
		${ENGINE_LIB_SRC_DIR}/ktx/include
		${ENGINE_LIB_SRC_DIR}/ktx/other_include
		${ENGINE_LIB_SRC_DIR}/cereal/include
		${ENGINE_LIB_SRC_DIR}/utf8/include
		${ENGINE_LIB_SRC_DIR}/zlib/src
		${ENGINE_LIB_SRC_DIR}/charset-detect
		${ENGINE_LIB_SRC_DIR}/LuaBridge
		${ENGINE_LIB_SRC_DIR}/lua/include
		${ENGINE_LIB_SRC_DIR}/libmono/6.13.0/include
		${ENGINE_LIB_SRC_DIR}/tracy
	)

	target_link_libraries(MapleEngine 
		${ENGINE_LIB_SRC_DIR}/opengl/lib/${Arch}/glfw3.lib
		${ENGINE_LIB_SRC_DIR}/opengl/lib/${Arch}/opengl32.lib
		${ENGINE_LIB_SRC_DIR}/opengl/lib/${Arch}/glew.lib
		${ENGINE_LIB_SRC_DIR}/vulkan/lib/${Arch}/vulkan-1.lib
		${ENGINE_LIB_SRC_DIR}/vulkan/lib/${Arch}/VkLayer_utils.lib
		${ENGINE_LIB_SRC_DIR}/libmono/6.13.0/windows/${Arch}/libmono-static-sgen.lib
		tinyobjloader
		#imgui
		spirvCross
		ktx
		tellenc
		zlib
		lua
		ws2_32
		Winmm
		Version
		Bcrypt
		TracyClient
	)

	set_target_properties(lua zlib tinyobjloader tellenc spirvCross ktx spdlog_headers_for_ide stb_image TracyClient PROPERTIES FOLDER Library)
	

endif()


