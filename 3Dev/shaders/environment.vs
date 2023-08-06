#version 450 core
layout (location = 0) in vec3 position;

out vec3 outpos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    outpos = vec3(model * vec4(position, 1.0));
    gl_Position = projection * view * model * vec4(position, 1.0);
}
