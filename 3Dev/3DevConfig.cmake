get_filename_component(3DEV_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(3DEV_INCLUDE_DIR "${3DEV_DIR}/../../../include/3Dev" ABSOLUTE)

set(3DEV_DEPS
    LightLog
    reactphysics3d
    GL
    GLEW
    assimp
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sfml-network
    jsoncpp
)
