#version 450 core

const vec2 invatan = vec2(0.1591, 0.3183);

uniform sampler2D environment;

in vec3 outpos;

out vec4 color;

vec2 SampleMap(vec3 vec)
{
    vec2 uv = -vec2(atan(vec.z, vec.x), asin(vec.y));
    uv *= invatan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 coord = SampleMap(normalize(outpos)); 
    color = texture(environment, coord);
}
