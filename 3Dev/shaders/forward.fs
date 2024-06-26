#version 450 core

const int maxLights = 64;
const int maxShadows = 8;
const int maxLodLevel = 7;
const float pi = 3.14159265;

uniform sampler2D albedo;
uniform sampler2D normalMap;
uniform sampler2D ao;
uniform sampler2D metalness;
uniform sampler2D emission;
uniform sampler2D roughness;
uniform sampler2D opacity;
uniform samplerCube irradiance;
uniform samplerCube prefilteredMap;
uniform sampler2D lut;

uniform vec3 nalbedo;
uniform bool nnormalMap;
uniform vec3 nemission;
uniform float nmetalness;
uniform float nroughness;
uniform bool nao;
uniform float nopacity;
uniform vec3 nirradiance;

uniform bool drawTransparency = false;
uniform float emissionStrength = 1.0;
uniform float shadowBias;
uniform mat4 invView;

in vec2 coord;
in vec3 camposout;
in vec3 mnormal;
in vec3 mpos;
in mat3 tbn;
in vec4 lspaceout[maxShadows];

out vec4 color;

struct Shadow
{
	vec3 sourcepos;
	sampler2DShadow shadowMap;
    bool isactive;
};

struct Light
{
    vec3 color;
    vec3 position;
    vec3 direction;
    vec3 attenuation;

    float cutoff;
    float outerCutoff;

    bool isactive;
    bool castShadows;
};

uniform Light lights[maxLights];
uniform Shadow shadows[maxShadows];

vec3 pos;

float CalcShadow()
{
    float ret = 0.0;
    for(int i = 0; i < maxShadows; i++)
    {
        if(!shadows[i].isactive) continue;

        vec3 pcoord = lspaceout[i].xyz / lspaceout[i].w;
        pcoord = pcoord * 0.5 + 0.5;
        if(pcoord.z > 1.0) continue;
        
        pcoord.z -= shadowBias;
        float tmp = 1.0 - texture(shadows[i].shadowMap, pcoord);
        if(ret < tmp) ret = tmp;
    }
    return ret;
}

float GGX(float ndoth, float rough)
{
    float dn = pi * pow(pow(ndoth, 2.0) * (pow(rough, 4.0) - 1.0) + 1.0, 2.0);

    return pow(rough, 4.0) / dn;
}

float GeometrySchlick(float ndotv, float rough)
{
    float k = pow(rough + 1.0, 2.0) / 8.0;
    float dn = ndotv * (1.0 - k) + k;

    return ndotv / dn;
}

float GeometrySmith(float ndotv, float ndotl, float rough)
{
    float ggx1  = GeometrySchlick(ndotv, rough);
    float ggx2  = GeometrySchlick(ndotl, rough);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTh, vec3 f0, float rough)
{
    return f0 + (max(vec3(1.0 - rough), f0) - f0) * pow(1.0 - cosTh, 5.0);
}

vec3 CalcLight(Light light, vec3 norm, float rough, float metal, vec3 albedo, vec3 irr, vec3 f0)
{
    float theta = dot(normalize(light.position - pos), normalize(-light.direction));
    float intensity = 1.0;
    if(light.cutoff != 1.0)
        intensity = clamp((theta - light.outerCutoff) / (light.cutoff - light.outerCutoff), 0.0, 1.0);
    if(theta < light.cutoff && intensity <= 0.0) return vec3(0.0);

    vec3 v = normalize(camposout - pos);

    vec3 l = (light.cutoff == 1.0 ? normalize(light.position - pos) : normalize(-light.direction));
    vec3 h = normalize(v + l);

    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * length(light.position - pos) + light.attenuation.z * pow(length(light.position - pos), 2));

    float ndoth = max(dot(norm, h), 0.0);
    float ndotv = max(dot(norm, v), 0.0);
    float ndotl = max(dot(norm, l), 0.0);

    //attenuation = 1.0 / pow(length(l), 2.0);

    vec3 rad = light.color * attenuation * intensity;

    float ndf = GGX(ndoth, rough);
    float g = GeometrySmith(ndotv, ndotl, rough);
    vec3 f = FresnelSchlick(max(dot(h, v), 0.0), f0, rough);

    vec3 kspc = f;
    vec3 kdif = (1.0 - kspc) * (1.0 - metal);

    vec3 nm = ndf * g * f;
    float dn = 4.0 * ndotv * ndotl;
    vec3 spc = (nm / max(dn, 0.001));

    vec3 lo = (kdif * albedo / pi + spc) * rad * ndotl;

    return lo;
}

void main()
{
    float alpha = (nopacity < 0.0 ? texture(opacity, coord).x : nopacity) * (nalbedo.x < 0.0 ? texture(albedo, coord).w : 1.0);

    if((drawTransparency && alpha >= 1.0) || (!drawTransparency && alpha < 1.0)) discard;

    vec3 norm;
    if(nnormalMap) norm = mat3(invView) * normalize(tbn * normalize(texture(normalMap, coord).xyz * 2.0 - 1.0));
    else norm = mat3(invView) * normalize(mnormal);

    pos = (invView * vec4(mpos, 1.0)).xyz;

    vec3 emission = (nemission.x < 0.0 ? texture(emission, coord).xyz : nemission) * emissionStrength;
    float rough = (nroughness < 0.0 ? texture(roughness, coord).x : nroughness);
    float metal = (nmetalness < 0.0 ? texture(metalness, coord).x : nmetalness);
    float ao = (nao ? texture(ao, coord).x : 1.0);
    vec3 alb = (nalbedo.x < 0.0 ? texture(albedo, coord).xyz : nalbedo);
    vec3 irr = (nirradiance.x < 0.0 ? texture(irradiance, norm).xyz : nirradiance);
    vec3 prefiltered = textureLod(prefilteredMap, reflect(-normalize(camposout - pos), norm), rough * maxLodLevel).xyz;

    vec3 total = vec3(0.0), totalNoShadow = vec3(0.0);
    float shadow = 0.0;
    vec3 f0 = mix(vec3(0.04), alb, metal);
    int i = 0;
    while(lights[i].isactive)
    {
        if(lights[i].castShadows)
            total += CalcLight(lights[i], norm, rough, metal, alb, irr, f0);
        else totalNoShadow += CalcLight(lights[i], norm, rough, metal, alb, irr, f0);
        i++;
    }
    shadow = CalcShadow();

    vec3 f = FresnelSchlick(max(dot(norm, normalize(camposout - pos)), 0.0), f0, rough);
    vec3 kspc = f;
    vec3 kdif = (1.0 - kspc) * (1.0 - metal);

    vec2 brdf = normalize(texture(lut, vec2(max(dot(norm, normalize(camposout - pos)), 0.0), rough)).xy);
    vec3 spc = prefiltered * (f * brdf.x + brdf.y);

    vec3 diffuse = irr * alb;
    vec3 ambient = (kdif * diffuse) + spc;

    float shadowCoef = (length(emission) > 0.0 ? 1.0 : (1.0 - shadow));
    total += ambient;
    color = vec4(total * ao * shadowCoef + ((ambient / 5.0) * (1.0 - shadowCoef) * ao) + emission + (totalNoShadow * ao), alpha);
}
