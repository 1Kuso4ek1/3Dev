#version 330

in vec2 coord;

uniform vec2 pixelsize;
uniform sampler2D frame;
uniform float exposure = 1.0;

out vec4 color;

void main()
{
    color = texture(frame, coord);
    color.rgb = color.rgb / (color.rgb + vec3(1.0));
    color.rgb *= exposure;
    
    float lum = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    float mlum = (lum * (1.0 + lum)) / (1.0 + lum);

    color.rgb = (mlum / lum) * color.rgb;
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
}
