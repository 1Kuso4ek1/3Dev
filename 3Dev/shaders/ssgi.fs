#version 450 core

uniform vec2 pixelsize;
uniform sampler2D gposition;
uniform sampler2D galbedo;

in vec2 coord;

out vec4 color;

const int samples = 2;

float rand(vec2 v)
{
    return fract(sin(dot(v, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec3 pos = texture(gposition, coord).xyz;
    if(pos.z == 0.0) discard;

    float randAngle = rand(coord);
    float randSample = -0.7 * rand(coord.yx) + 1.0;
    vec3 gi = vec3(0.0);

    for(int i = 0; i < samples; i++)
    {
        float sampleDistance = exp(i - samples) * randSample;
        float phi = ((i + randAngle * samples) * 4.0 * 3.14159265) / samples;
        vec2 uv = sampleDistance * vec2(cos(phi), sin(phi));

        vec3 lightColor = texture(galbedo, coord + uv).rgb;

        vec3 lightPosition = texture(gposition, coord + uv).xyz;

        gi += lightColor * (1.0 - clamp(pow(length(lightPosition - pos) / 64.0, 4.0), 0.0, 1.0));
    }

    color = vec4(max(gi / samples, vec3(0.000001)), 1.0);
}
