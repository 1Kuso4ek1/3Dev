#version 330
layout (location = 0) in vec3 position;

uniform mat4 light;
uniform mat4 model;

void main()
{
    gl_Position = light * model * vec4(position, 1.0);
}
