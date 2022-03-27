cmake_minimum_required(VERSION 3.20)

project(3Dev VERSION 1.0.0 LANGUAGES CXX)

find_package(SFML 2.5.1 COMPONENTS graphics system window network audio REQUIRED)
find_package(assimp REQUIRED)
find_package(GLEW 2.2.0 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ImGui-SFML REQUIRED)
find_package(ReactPhysics3D REQUIRED)

add_library(3Dev 
	src/Matrices.cpp
	src/Camera.cpp
	src/Light.cpp
	src/Model.cpp
	src/Shader.cpp
	src/Shape.cpp
	src/Material.cpp
	src/Engine.cpp
	src/Framebuffer.cpp
	src/Mesh.cpp
	src/PhysicsManager.cpp
)

add_executable(physics physics_test.cpp)
					
target_link_libraries(3Dev PUBLIC reactphysics3d GL GLEW GLU assimp pthread sfml-graphics sfml-window sfml-system sfml-audio sfml-network ImGui-SFML)
target_link_libraries(physics PUBLIC 3Dev reactphysics3d GL GLU GLEW assimp pthread sfml-graphics sfml-window sfml-system sfml-audio sfml-network ImGui-SFML)
set_target_properties(3Dev PROPERTIES C_STANDARD 17 C_STANDARD_REQUIRED ON)

target_include_directories(3Dev 
	PUBLIC
	/usr/include
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

target_include_directories(physics
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

install(TARGETS 3Dev 
	EXPORT 3Dev
	DESTINATION lib
)
		
install(FILES 3DevConfig.cmake
	DESTINATION lib/cmake/3Dev
)

install(FILES 
	include/3Dev.h
	include/Matrices.h
	include/Camera.h
	include/Engine.h
	include/Light.h
	include/Material.h
	include/Model.h
	include/Physics.h
	include/Shader.h
	include/Shape.h
	include/Texture.h
	include/Framebuffer.h
	include/Mesh.h
	include/PhysicsManager.h
	DESTINATION	include/3Dev
)
