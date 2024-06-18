#version 450 core

uniform sampler2D tex;
uniform sampler2D prev;
uniform float adaptSpeed = 0.02;

in vec2 coord;

out vec4 color;

void main()
{
    float old = texture(prev, coord, 12.0).r;
    float current = dot(vec3(0.2125, 0.7154, 0.0721), texture(tex, coord).rgb);

    color = vec4(old + (current - old) * adaptSpeed, 0.0, 0.0, 1.0);
}
