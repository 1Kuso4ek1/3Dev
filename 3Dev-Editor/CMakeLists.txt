cmake_minimum_required(VERSION 3.17)

project(editor VERSION 0.1.0 LANGUAGES CXX)

find_package(3Dev REQUIRED)
find_package(TGUI REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ReactPhysics3D REQUIRED)
find_package(glm REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(Angelscript REQUIRED)
find_package(Threads REQUIRED)

add_executable(editor main.cpp)

target_include_directories(editor 
  	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(editor PUBLIC 3Dev::3Dev TGUI::TGUI stdc++fs)

#install(TARGETS editor DESTINATION bin)
install(DIRECTORY DESTINATION $ENV{HOME}/.3Dev-Editor)
install(DIRECTORY DESTINATION $ENV{HOME}/.3Dev-Editor/default)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/default/hdri.hdr DESTINATION $ENV{HOME}/.3Dev-Editor/default)
