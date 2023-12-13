#version 450 core
precision mediump float;

uniform vec2 pixelsize;
uniform sampler2D gposition;
uniform sampler2D gnormal;
uniform sampler2D noise;

uniform int numSamples;

uniform float radius;
uniform float strength;

uniform vec3 samples[128];
uniform mat4 projection;

in vec2 coord;

out float color;

void main()
{
    if(texture(gposition, coord).w < 1.0)
    {
        color = 1.0;
        return;
    }
    vec3 pos = texture(gposition, coord).xyz;
    vec3 normal = texture(gnormal, coord).xyz;
    vec3 random = texture(noise, coord * pixelsize * 1.0).xyz;

    vec3 t = normalize(random - normal * dot(random, normal));
    vec3 b = cross(normal, t);
    mat3 tbn = mat3(t, b, normal);

    float occlusion = 0.0;
    for(int i = 0; i < numSamples; i++)
    {
        vec3 samplePos = tbn * samples[i];
        samplePos = pos + samplePos * radius;
        
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = texture(gposition, offset.xy).z;
        if(sampleDepth == 0.0) continue;

        occlusion += (sampleDepth >= samplePos.z + 0.1 ? 1.0 : 0.0) * smoothstep(0.0, 1.0, radius / abs(pos.z - sampleDepth));
    }

    color = pow(1.0 - (occlusion / numSamples), strength);
}