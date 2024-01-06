cmake_minimum_required(VERSION 3.17)

project(editor VERSION 1.3.6 LANGUAGES CXX)

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

option(3DEV_RELEASE OFF)

if(UNIX OR APPLE)
	find_package(GLEW 2.1.0 REQUIRED)
elseif("${GLEW_INCLUDE_DIR}" STREQUAL "" OR "${GLEW_LIBRARIES}" STREQUAL "")
	message(FATAL_ERROR "On Windows GLEW couldn't be found automatically, so you have to set GLEW_INCLUDE_DIR and GLEW_LIBRARIES manually")
endif()

add_executable(editor main.cpp)

add_definitions("-DEDITOR_VERSION=\"1.3.8-1\"")

if(3DEV_RELEASE)
	add_definitions("-DSHADERS_DIRECTORY=\"../../shaders/\"")
else()
	add_definitions("-DSHADERS_DIRECTORY=\"${CMAKE_CURRENT_SOURCE_DIR}/../3Dev/shaders/\"")
endif()

target_include_directories(editor
  	PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../3Dev/libraries/assimp/include>
	$<BUILD_INTERFACE:${SFML_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${LIGHTLOG_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${GLEW_INCLUDE_DIR}>
)

target_link_directories(editor
	PUBLIC
	${GLEW_LIBRARIES}
	${SFML_INCLUDE_DIR}/../lib
	${LIGHTLOG_INCLUDE_DIR}/../lib
)

target_link_libraries(editor PUBLIC 3Dev::3Dev TGUI::TGUI)

if(UNIX OR APPLE)
    file(TO_CMAKE_PATH $ENV{HOME} HOME_PATH)
else()
    file(TO_CMAKE_PATH $ENV{HOMEPATH} HOME_PATH)
endif()

install(DIRECTORY DESTINATION ${HOME_PATH}/.3Dev-Editor)
install(DIRECTORY DESTINATION ${HOME_PATH}/.3Dev-Editor/gui)
install(DIRECTORY DESTINATION ${HOME_PATH}/.3Dev-Editor/log)
install(DIRECTORY DESTINATION ${HOME_PATH}/.3Dev-Editor/default)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/default/hdri.hdr DESTINATION ${HOME_PATH}/.3Dev-Editor/default)
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/menu.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/loading.txt 
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/editor.txt
    DESTINATION ${HOME_PATH}/.3Dev-Editor/gui
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/themes/Black.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/themes/Black.png
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/themes/SourceCodePro-Regular.ttf
    DESTINATION ${HOME_PATH}/.3Dev-Editor/gui/themes
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/icon.png
    DESTINATION ${HOME_PATH}/.3Dev-Editor
)

if(UNIX)
    write_file(${CMAKE_CURRENT_BINARY_DIR}/3Dev-Editor.desktop
    "[Desktop Entry]\nVersion=1.0\nType=Application\nName=3Dev Editor\n\
Keywords=game;engine;\nCategories=Programming;\nTerminal=false\n\
Exec=\"${CMAKE_CURRENT_BINARY_DIR}/editor\"\nIcon=icon\n\
Comment=Create a game."
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/3Dev-Editor.desktop DESTINATION ${HOME_PATH}/.local/share/applications)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icon.png DESTINATION ${HOME_PATH}/.local/share/icons)
endif()
