cmake_minimum_required(VERSION 3.17)

project(render VERSION 1.0.0 LANGUAGES CXX)

find_package(3Dev REQUIRED)
find_package(SFML 3.0.0 COMPONENTS System Window Network Audio REQUIRED)
find_package(TGUI REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ReactPhysics3D REQUIRED)
find_package(LightLog REQUIRED)
find_package(glm REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(Angelscript REQUIRED)
find_package(Threads REQUIRED)

if(UNIX OR APPLE)
	find_package(GLEW 2.1.0 REQUIRED)
elseif("${GLEW_INCLUDE_DIR}" STREQUAL "" OR "${GLEW_LIBRARIES}" STREQUAL "")
	message(FATAL_ERROR "On Windows GLEW couldn't be found automatically, so you have to set GLEW_INCLUDE_DIR and GLEW_LIBRARIES manually")
endif()

add_executable(render main.cpp)

target_include_directories(render
  	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../3Dev/libraries/assimp/include>
	$<BUILD_INTERFACE:${SFML_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${LIGHTLOG_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${GLEW_INCLUDE_DIR}>
)

target_link_directories(render
	PUBLIC
	${GLEW_LIBRARIES}
	${SFML_INCLUDE_DIR}/../lib
	${LIGHTLOG_INCLUDE_DIR}/../lib
)

target_link_libraries(render PUBLIC 3Dev::3Dev)

install(TARGETS render DESTINATION bin)

