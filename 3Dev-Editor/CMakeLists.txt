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

target_link_libraries(editor PUBLIC 3Dev::3Dev TGUI::TGUI stdc++fs)

install(DIRECTORY DESTINATION $ENV{HOME}/.3Dev-Editor)
install(DIRECTORY DESTINATION $ENV{HOME}/.3Dev-Editor/gui)
install(DIRECTORY DESTINATION $ENV{HOME}/.3Dev-Editor/default)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/default/hdri.hdr DESTINATION $ENV{HOME}/.3Dev-Editor/default)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gui/menu.txt ${CMAKE_CURRENT_SOURCE_DIR}/gui/editor.txt DESTINATION $ENV{HOME}/.3Dev-Editor/gui)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gui/themes/Black.txt ${CMAKE_CURRENT_SOURCE_DIR}/gui/themes/Black.png DESTINATION $ENV{HOME}/.3Dev-Editor/gui/themes)
if(UNIX)
    write_file(${CMAKE_CURRENT_SOURCE_DIR}/3Dev-Editor.desktop
    "[Desktop Entry]\nVersion=1.0\nType=Application\nName=3Dev Editor\n\
Keywords=game;engine;\nCategories=Programming;\nTerminal=false\n\
Exec=${CMAKE_CURRENT_BINARY_DIR}/editor\nIcon=${CMAKE_CURRENT_SOURCE_DIR}/icon.png\n\
Comment=Create a game."
    )
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/3Dev-Editor.desktop DESTINATION $ENV{HOME}/.local/share/applications)
endif()
