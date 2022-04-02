#version 330
precision mediump float;

in vec2 coord;

uniform vec2 pixelsize;
uniform sampler2D frame;

out vec4 color;

void main()
{
    vec2 pos = gl_FragCoord.xy * pixelsize;
    color = texture(frame, coord);
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
}
