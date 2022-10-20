cmake_minimum_required(VERSION 3.17)

project(render VERSION 1.0.0 LANGUAGES CXX)

find_package(3Dev REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ReactPhysics3D REQUIRED)
find_package(glm REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(Angelscript REQUIRED)
find_package(Threads REQUIRED)

add_executable(render main.cpp)

target_link_libraries(render PUBLIC 3Dev::3Dev)

install(TARGETS render DESTINATION bin)

