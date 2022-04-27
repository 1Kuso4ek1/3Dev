#version 330
precision mediump float;

const int maxlights = 16;

uniform sampler2D diff;
uniform sampler2D normalmap;
uniform sampler2D ao;
uniform sampler2D metalness;
uniform sampler2D emission;
uniform sampler2D roughness;
uniform sampler2D opacity;
uniform samplerCube cubemap;

uniform bool ndiff;
uniform bool nnormalmap;
uniform bool nemission;
uniform bool nmetalness;
uniform bool nroughness;
uniform bool nao;
uniform bool nopacity;

uniform float shininess;

in vec2 coord;
in vec3 camposout;
in vec3 mnormal;
in vec3 mpos;
in mat3 tbn;

out vec4 color;

struct Light
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
    vec3 attenuation;

    bool isactive;
};

uniform Light lights[maxlights];

vec3 CalcLight(Light l, vec3 norm)
{
    float dist = length(l.position - mpos);
    float attenuation = 1.0 / (l.attenuation.x + l.attenuation.y * dist + l.attenuation.z * dist * dist);
    
    vec3 dcolor = (ndiff ? texture(diff, coord).xyz : vec3(1.0));

    vec3 totalamb = dcolor * l.ambient;

    float dcontrib = max(0.0, dot(norm, normalize(l.position - mpos)));
    vec3 totaldif = dcolor * l.diffuse * dcontrib * (nroughness ? texture(roughness, coord).x : 1.0);
    
    vec3 halfway = normalize(normalize(l.position - mpos) + normalize(camposout - mpos));
    float scontrib = pow(max(0.0, dot(norm, halfway)), shininess * (nroughness ? 1.0 - texture(roughness, coord).x : 1.0));
    vec3 totalspc = dcolor * l.specular * scontrib * (nmetalness ? texture(metalness, coord).xyz : vec3(1.0)) * (nroughness ? 1.0 - texture(roughness, coord).x : 1.0);
    
    return (totalamb + totaldif + totalspc) * attenuation;
}

void main()
{
    vec3 norm;
    if(nnormalmap) norm = tbn * normalize(texture(normalmap, coord).xyz * 2.0 - 1.0);
    else norm = normalize(mnormal);

    vec3 reflected = reflect(normalize(mpos - camposout), normalize(norm));
    vec3 reflection = textureCube(cubemap, reflected).xyz * (nmetalness ? texture(metalness, coord).xyz : vec3(1.0));

    vec3 emission = nemission ? texture(emission, coord).xyz : vec3(0.0);
    vec3 totallight = vec3(0.0);
    int i = 0;
    while(lights[i].isactive)
    {
        totallight += CalcLight(lights[i], norm);
        i++;
    }
    color = vec4(emission + (reflection * totallight * vec3(1.5)) + totallight, (nopacity ? texture(opacity, coord).x : 1.0)) * (nao ? texture(ao, coord) : vec4(1.0));
}
