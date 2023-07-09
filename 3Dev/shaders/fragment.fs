#version 330
precision mediump float;

const int maxLights = 32;
const int maxLodLevel = 7;
const float pi = 3.14159265;
const float shadowBias = 0.0000001;

uniform sampler2D gposition;
uniform sampler2D galbedo;
uniform sampler2D gnormal;
uniform sampler2D gemission;
uniform sampler2D gcombined;
uniform sampler2D opacity;
uniform samplerCube irradiance;
uniform samplerCube prefilteredMap;
uniform sampler2D lut;

uniform float nopacity;
uniform vec3 nirradiance;

uniform vec3 campos;

uniform mat4 lspace;

uniform bool drawTransparency = false;

in vec2 coord;

vec3 pos;
vec3 norm;
vec4 lspaceout;

out vec4 color;

struct Shadow
{
	sampler2DShadow shadowmap;
	vec3 sourcepos;
    bool perspective;
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
uniform Shadow shadow;

float LinearizeDepth(float depth)
{
    if(shadow.perspective)
    {
        float z = depth * 2.0 - 1.0;
        return (2.0 * 0.01 * 1000.0) / (1000.0 + 0.01 - z * (1000.0 - 0.01));
    }
    return depth;
}

float CalcShadow()
{
    if(1.0 - dot(norm, normalize(shadow.sourcepos - pos)) >= 1.0) return 0.0;
    vec3 pcoord = lspaceout.xyz / lspaceout.w;
    pcoord = pcoord * 0.5 + 0.5;
    if(pcoord.z > 1.0)
        return 0.0;
    
    pcoord.z -= shadowBias;
    return 1.0 - texture(shadow.shadowmap, pcoord);
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

    vec3 v = normalize(campos - pos);

    vec3 l = (light.cutoff == 1.0 ? normalize(light.position - pos) : normalize(-light.direction));
    vec3 h = normalize(v + l);

    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * length(l) + light.attenuation.z * pow(length(l), 2));

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
    float alpha = texture(galbedo, coord).w;

    if(!drawTransparency && alpha < 1.0)
        return;
    if(drawTransparency && alpha == 1.0)
    {
        color = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    pos = texture(gposition, coord).xyz;
    vec3 alb = texture(galbedo, coord).xyz;
    norm = texture(gnormal, coord).xyz;
    vec3 emission = texture(gemission, coord).xyz;
    float metal = texture(gcombined, coord).x;
    float rough = texture(gcombined, coord).y;
    float ao = texture(gcombined, coord).z;
    vec3 irr = (nirradiance.x < 0.0 ? texture(irradiance, norm).xyz : nirradiance);
    vec3 prefiltered = textureLod(prefilteredMap, reflect(-normalize(campos - pos), norm), rough * maxLodLevel).xyz;

    lspaceout = lspace * vec4(pos, 1.0);

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

    vec3 f = FresnelSchlick(max(dot(norm, normalize(campos - pos)), 0.0), f0, rough);
    vec3 kspc = f;
    vec3 kdif = (1.0 - kspc) * (1.0 - metal);

    vec2 brdf = normalize(texture(lut, vec2(max(dot(norm, normalize(campos - pos)), 0.0), rough)).xy);
    vec3 spc = prefiltered * (f * brdf.x + brdf.y);

    vec3 diffuse = irr * alb;
    vec3 ambient = ((kdif * diffuse) + spc) * ao;

    total += ambient / 2;
    color = vec4((total * (length(emission) > 0.0 ? 1.0 : (1.0 - shadow)) + ambient / 2) + (emission * 5) + totalNoShadow, (alpha < 1.0 ? alpha + ((total.x + total.y, + total.z) / 3.0) * alpha : 1.0));
    //color = vec4(alpha, alpha, alpha, 1.0);
}
