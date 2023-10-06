#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 ids;
layout (location = 4) in vec4 weights;

out vec2 coord;

void main()
{
    coord = uv;
    gl_Position = vec4(position, 1.0);
}
