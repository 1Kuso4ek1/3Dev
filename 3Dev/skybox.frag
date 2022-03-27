#version 300 es
precision mediump float;

uniform samplerCube cubemap;

in vec3 vertex;

out vec4 color;

void main()
{
    color = textureCube(cubemap, vertex);
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
}
