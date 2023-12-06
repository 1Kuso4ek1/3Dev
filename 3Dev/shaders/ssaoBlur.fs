#version 450 core
precision mediump float;

in vec2 coord;

uniform vec2 pixelsize;
uniform sampler2D ssao;

out float color;

void main()
{
    float result = 0.0;
    for(float x = -2.0; x < 2.0; x += 1.0)
        for(float y = -2.0; y < 2.0; y += 1.0)
            result += texture(ssao, coord + vec2(x, y) * pixelsize).r;

    color = result / 16.0;
}
