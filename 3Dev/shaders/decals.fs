#version 450 core

uniform sampler2D gposition;

uniform sampler2D albedo;
uniform sampler2D normalMap;
uniform sampler2D ao;
uniform sampler2D metalness;
uniform sampler2D emission;
uniform sampler2D roughness;
uniform sampler2D opacity;

uniform float shadowBias;
uniform vec3 nalbedo;
uniform bool nnormalMap;
uniform vec3 nemission;
uniform float emissionStrength;
uniform float nmetalness;
uniform float nroughness;
uniform bool nao;
uniform float nopacity;

in mat4 invModel;

in vec2 coord;
in vec4 mpos;

layout (location = 0) out vec4 galbedo;
layout (location = 1) out vec4 gnormal;
layout (location = 2) out vec4 gemission;
layout (location = 3) out vec4 gcombined;

void main()
{
    vec2 uv = (mpos.xy / mpos.w) * 0.5 + 0.5;

    vec4 pos = invModel * vec4(texture(gposition, uv).xyz, 1.0);
    vec3 bounds = 1.0 - abs(pos.xyz);
    if(bounds.x < 0.0 || bounds.y < 0.0 || bounds.z < 0.0)
        discard;

    vec2 decalUv = pos.xz * 0.5 + 0.5;
    decalUv.x = 1.0 - decalUv.x;

    vec3 emission = (nemission.x < 0.0 ? texture(emission, decalUv).xyz : nemission);
    float rough = (nroughness < 0.0 ? texture(roughness, decalUv).x : nroughness);
    float metal = (nmetalness < 0.0 ? texture(metalness, decalUv).x : nmetalness);
    float ao = (nao ? texture(ao, decalUv).x : 1.0);
    float alpha = (nopacity < 0.0 ? texture(opacity, decalUv).x : nopacity) * texture(albedo, decalUv).w;

    vec3 norm = vec3(0.0);

    galbedo = vec4(nalbedo.x < 0.0 ? texture(albedo, decalUv).xyz : nalbedo, alpha);
    gnormal = vec4(norm, 1.0);
    gemission = vec4(emission * emissionStrength, 1.0);
    gcombined = vec4(metal, rough, ao, shadowBias);
}