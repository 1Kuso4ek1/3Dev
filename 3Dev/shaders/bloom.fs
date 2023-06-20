#version 330
precision mediump float;

const float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

in vec2 coord;

uniform bool horizontal;
uniform vec2 pixelsize;
uniform sampler2D frame;

out vec4 color;

void main()
{
    vec3 ret = texture(frame, coord).xyz * weight[0];

    for(int i = 1; i < 5; i++)
    {
        ret += texture(frame, coord + vec2((horizontal ? pixelsize.x * i : 0.0), (horizontal ? 0.0 : pixelsize.y * i))).xyz * weight[i];
        ret += texture(frame, coord - vec2((horizontal ? pixelsize.x * i : 0.0), (horizontal ? 0.0 : pixelsize.y * i))).xyz * weight[i];
    }

    color = vec4(ret, 1.0);
}
