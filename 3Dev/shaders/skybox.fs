#version 450 core
precision mediump float;

uniform samplerCube cubemap;

in vec3 vertex;

out vec4 color;

void main()
{
    color = texture(cubemap, vertex);
}
