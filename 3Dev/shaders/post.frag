#version 330
precision mediump float;

in vec2 coord;

uniform vec2 pixelsize;
uniform sampler2D frame;
uniform float exposure = 1.0;

out vec4 color;

void main()
{
    color = vec4(1.0) - exp(-texture(frame, coord) * exposure);
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
}
