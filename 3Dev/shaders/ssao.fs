#version 450 core
precision mediump float;

uniform vec2 pixelsize;
uniform sampler2D gposition;
uniform sampler2D gnormal;
uniform sampler2D noise;

uniform vec3 samples[64];
uniform mat4 projection;

in vec2 coord;

out float color;

void main()
{
    vec3 pos = texture(gposition, coord).xyz;
    vec3 normal = texture(gnormal, coord).xyz;
    vec3 random = texture(noise, coord * pixelsize * 100000.0).xyz;

    vec3 t = normalize(random - normal * dot(random, normal));
    vec3 b = cross(normal, t);
    mat3 tbn = mat3(t, b, normal);

    float occlusion = 0.0;
    for(int i = 0; i < 64; i++)
    {
        vec3 samplePos = tbn * samples[i];
        samplePos = pos + samplePos * 0.5;
        
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = texture(gposition, offset.xy).z;

        occlusion += (sampleDepth >= samplePos.z + 0.025 ? 1.0 : 0.0) * smoothstep(0.0, 1.0, 0.5 / abs(pos.z - sampleDepth));
    }

    color = 1.0 - (occlusion / 64.0);
}
