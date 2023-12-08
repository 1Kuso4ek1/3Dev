#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out mat4 invModel;

out vec2 coord;
out vec4 mpos;

void main()
{
    vec4 pos = vec4(position, 1.0);

    coord = uv;
    invModel = inverse(model);

    mpos = projection * view * model * pos;
    gl_Position = mpos;
}
