cmake_minimum_required(VERSION 3.17)

project(3Dev VERSION 1.1.0 LANGUAGES CXX)

find_package(SFML 2.5.1 COMPONENTS system window network audio REQUIRED)
find_package(assimp REQUIRED)
find_package(GLEW 2.1.0 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ReactPhysics3D REQUIRED)
find_package(LightLog REQUIRED)
find_package(glm REQUIRED)
find_package(jsoncpp REQUIRED)

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
	src/SceneManager.cpp
	src/ShadowManager.cpp
	src/SoundManager.cpp
	src/Renderer.cpp
)

add_definitions("-DSHADERS_DIRECTORY=\"${CMAKE_CURRENT_SOURCE_DIR}/shaders/\"")

add_executable(physics physics_test.cpp)
          
target_link_libraries(3Dev PUBLIC LightLog ReactPhysics3D::ReactPhysics3D OpenGL::GL GLEW glm::glm assimp::assimp sfml-graphics sfml-window sfml-system sfml-audio sfml-network jsoncpp_lib)

target_link_libraries(physics PUBLIC 3Dev)

set_target_properties(3Dev PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
set_target_properties(physics PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_include_directories(3Dev 
  	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${SFML_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${LIGHTLOG_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${GLEW_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:include>
)

target_link_directories(physics
	PUBLIC
	${GLEW_LIBRARIES}
	${SFML_INCLUDE_DIR}/../lib
	${LIGHTLOG_INCLUDE_DIR}/../lib
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
  	include/Utils.hpp
  	include/Matrices.hpp
  	include/Camera.hpp
  	include/Engine.hpp
  	include/Light.hpp
  	include/Material.hpp
  	include/Model.hpp
  	include/Shader.hpp
  	include/Shape.hpp
  	include/Texture.hpp
  	include/Framebuffer.hpp
  	include/Mesh.hpp
  	include/PhysicsManager.hpp
	include/SceneManager.hpp
	include/ShadowManager.hpp
	include/SoundManager.hpp
	include/Renderer.hpp
	include/stb_image.hpp
	DESTINATION include/3Dev
)
