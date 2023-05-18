cmake_minimum_required(VERSION 3.17)

project(3Dev VERSION 1.3.0 LANGUAGES CXX)

find_package(SFML 2.5.1 COMPONENTS system window network audio REQUIRED)
find_package(TGUI REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ReactPhysics3D REQUIRED)
find_package(LightLog REQUIRED)
find_package(glm REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(Angelscript REQUIRED)
find_package(Threads REQUIRED)

set(ANGELSCRIPT_LIBRARY_NAME angelscript)
option(WIN_RELEASE OFF)

if(UNIX OR APPLE)
	find_package(GLEW 2.1.0 REQUIRED)
	if(APPLE)
		set(ANGELSCRIPT_LIBRARY_NAME Angelscript)
	endif()
elseif("${GLEW_INCLUDE_DIR}" STREQUAL "" OR "${GLEW_LIBRARIES}" STREQUAL "")
	message(FATAL_ERROR "On Windows GLEW couldn't be found automatically, so you have to set GLEW_INCLUDE_DIR and GLEW_LIBRARIES manually")
endif()

include(CMakePackageConfigHelpers)

add_library(3Dev
	src/Matrices.cpp
	src/Camera.cpp
	src/Light.cpp
	src/Model.cpp
	src/Shader.cpp
	src/Material.cpp
	src/Engine.cpp
	src/Framebuffer.cpp
	src/Mesh.cpp
	src/PhysicsManager.cpp
	src/SceneManager.cpp
	src/ShadowManager.cpp
	src/SoundManager.cpp
	src/Renderer.cpp
	src/ScriptManager.cpp
	src/TextureManager.cpp
	src/Node.cpp
	src/Bone.cpp
	src/Animation.cpp
	libraries/Angelscript/sdk/add_on/scriptbuilder/scriptbuilder.cpp
	libraries/Angelscript/sdk/add_on/scriptstdstring/scriptstdstring.cpp
	libraries/Angelscript/sdk/add_on/scriptarray/scriptarray.cpp
	libraries/Angelscript/sdk/add_on/scriptmath/scriptmath.cpp
	libraries/Angelscript/sdk/add_on/datetime/datetime.cpp
	libraries/Angelscript/sdk/add_on/scriptdictionary/scriptdictionary.cpp
	libraries/Angelscript/sdk/add_on/scriptfile/scriptfile.cpp
)

if(WIN_RELEASE)
	add_definitions("-DSHADERS_DIRECTORY=\"../../shaders/\"")
else()
	add_definitions("-DSHADERS_DIRECTORY=\"${CMAKE_CURRENT_SOURCE_DIR}/shaders/\"")
endif()

add_executable(launcher launcher.cpp)

target_link_libraries(3Dev
	PUBLIC
	LightLog
	ReactPhysics3D::ReactPhysics3D
	OpenGL::GL
	GLEW
	glm::glm
	assimp::assimp
	Angelscript::${ANGELSCRIPT_LIBRARY_NAME}
	TGUI::TGUI
	sfml-graphics sfml-window sfml-system sfml-audio sfml-network
	jsoncpp_lib
	stdc++fs
)

set_target_properties(3Dev PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
set_target_properties(launcher PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_include_directories(3Dev
  	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${SFML_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${LIGHTLOG_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${GLEW_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libraries/stb>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libraries/Angelscript/sdk/add_on/scriptbuilder>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libraries/Angelscript/sdk/add_on/scriptstdstring>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libraries/Angelscript/sdk/add_on/scriptarray>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libraries/Angelscript/sdk/add_on/autowrapper>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libraries/Angelscript/sdk/add_on/scriptmath>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libraries/Angelscript/sdk/add_on/datetime>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libraries/Angelscript/sdk/add_on/scriptdictionary>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libraries/Angelscript/sdk/add_on/scriptfile>
	$<INSTALL_INTERFACE:include>
)

target_link_directories(3Dev
	PUBLIC
	${GLEW_LIBRARIES}
	${SFML_INCLUDE_DIR}/../lib
	${LIGHTLOG_INCLUDE_DIR}/../lib
)

target_link_libraries(launcher PUBLIC 3Dev)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/3DevConfig.cmake
	INSTALL_DESTINATION lib/cmake/3Dev
)

install(TARGETS 3Dev
  	EXPORT 3DevTargets
  	LIBRARY DESTINATION lib
	INCLUDES DESTINATION include/3Dev
)

install(EXPORT 3DevTargets
	FILE 3DevTargets.cmake
	NAMESPACE 3Dev::
  	DESTINATION lib/cmake/3Dev
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/3DevConfig.cmake
	DESTINATION lib/cmake/3Dev
)

install(FILES
  	include/Utils.hpp
	include/AngelscriptUtils.hpp
  	include/Matrices.hpp
  	include/Camera.hpp
  	include/Engine.hpp
  	include/Light.hpp
  	include/Material.hpp
  	include/Model.hpp
  	include/Shader.hpp
  	include/Framebuffer.hpp
  	include/Mesh.hpp
  	include/PhysicsManager.hpp
	include/SceneManager.hpp
	include/ShadowManager.hpp
	include/SoundManager.hpp
	include/Renderer.hpp
	include/ScriptManager.hpp
	include/TextureManager.hpp
	include/Node.hpp
	include/Bone.hpp
	include/Animation.hpp
	libraries/stb/stb_image.h
	libraries/Angelscript/sdk/add_on/scriptbuilder/scriptbuilder.h
	libraries/Angelscript/sdk/add_on/scriptstdstring/scriptstdstring.h
	libraries/Angelscript/sdk/add_on/scriptarray/scriptarray.h
	libraries/Angelscript/sdk/add_on/autowrapper/aswrappedcall.h
	libraries/Angelscript/sdk/add_on/scriptmath/scriptmath.h
	libraries/Angelscript/sdk/add_on/datetime/datetime.h
	libraries/Angelscript/sdk/add_on/scriptdictionary/scriptdictionary.h
	libraries/Angelscript/sdk/add_on/scriptfile/scriptfile.h
	DESTINATION include/3Dev
)
