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
)

add_executable(physics physics_test.cpp)
          
target_link_libraries(3Dev PUBLIC LightLog reactphysics3d GL GLEW assimp sfml-graphics sfml-window sfml-system sfml-audio sfml-network jsoncpp_lib)

if(UNIX OR APPLE)
	target_link_libraries(3Dev PUBLIC GL)
elseif(WIN32)
	target_link_libraries(3Dev PUBLIC opengl32)
endif()

target_link_libraries(physics PUBLIC 3Dev)

set_target_properties(3Dev PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

get_target_property(ASSIMP_INCLUDE_DIR assimp::assimp INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(GLM_INCLUDE_DIR glm::glm INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(RP3D_INCLUDE_DIR ReactPhysics3D::ReactPhysics3D INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(JSONCPP_INCLUDE_DIR jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(3Dev 
  	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${SFML_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${LIGHTLOG_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${RP3D_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${GLEW_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${GLM_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${ASSIMP_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${JSONCPP_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:include>
)

target_link_directories(physics
	PUBLIC
	${GLEW_LIBRARIES}
	${RP3D_INCLUDE_DIR}/../lib
	${ASSIMP_INCLUDE_DIR}/../lib
	${SFML_INCLUDE_DIR}/../lib
	${LIGHTLOG_INCLUDE_DIR}/../lib
	${JSONCPP_INCLUDE_DIR}/../lib
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
  	include/3Dev.hpp
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
	include/stb_image.hpp
	DESTINATION include/3Dev
)
