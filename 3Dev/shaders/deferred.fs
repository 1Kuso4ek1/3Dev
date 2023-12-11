#version 450 core

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

in vec2 coord;
in vec3 mnormal;
in vec3 mpos;
in mat3 tbn;

layout (location = 0) out vec4 gposition;
layout (location = 1) out vec4 galbedo;
layout (location = 2) out vec4 gnormal;
layout (location = 3) out vec4 gemission;
layout (location = 4) out vec4 gcombined;

void main()
{
    float alpha = (nopacity < 0.0 ? texture(opacity, coord).x : nopacity) * texture(albedo, coord).w;
    
    if(alpha < 1.0) return;

    vec3 norm;
    if(nnormalMap) norm = normalize(tbn * normalize(texture(normalMap, coord).xyz * 2.0 - 1.0));
    else norm = normalize(mnormal);

    vec3 emission = (nemission.x < 0.0 ? texture(emission, coord).xyz : nemission);
    float rough = (nroughness < 0.0 ? texture(roughness, coord).x : nroughness);
    float metal = (nmetalness < 0.0 ? texture(metalness, coord).x : nmetalness);
    float ao = (nao ? texture(ao, coord).x : 1.0);
    vec3 alb = (nalbedo.x < 0.0 ? texture(albedo, coord).xyz : nalbedo);

    gposition = vec4(mpos, 1.0);
    galbedo = vec4(alb, alpha);
    gnormal = vec4(norm, 1.0);
    gemission = vec4(emission * emissionStrength, 1.0);
    gcombined = vec4(metal, rough, ao, shadowBias);
}