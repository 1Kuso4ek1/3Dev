#version 450 core

uniform samplerCube cubemap;

in vec3 vertex;

out vec4 color;

void main()
{
    color = texture(cubemap, vertex);
}
