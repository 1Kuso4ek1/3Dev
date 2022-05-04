#version 330
precision mediump float;

const int maxlights = 16;
const int maxshadows = 8;
const float pi = 3.14159265;

uniform sampler2D shadowmap;

uniform sampler2D albedo;
uniform sampler2D normalmap;
uniform sampler2D ao;
uniform sampler2D metalness;
uniform sampler2D emission;
uniform sampler2D roughness;
uniform sampler2D opacity;
uniform samplerCube cubemap;

uniform vec3 nalbedo;
uniform bool nnormalmap;
uniform vec3 nemission;
uniform float nmetalness;
uniform float nroughness;
uniform bool nao;
uniform float nopacity;

in vec2 coord;
in vec3 camposout;
in vec3 mnormal;
in vec3 mpos;
in mat3 tbn;
in vec4 lspaceout[maxshadows];

out vec4 color;

struct Shadow
{
	sampler2D shadowmap;
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
};

uniform Light lights[maxlights];
uniform Shadow shadows[maxshadows];

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * 0.01 * 500.0) / (500.0 + 0.01 - z * (500.0 - 0.01));
}

float CalcShadow(int i)
{
    vec3 pcoord = lspaceout[i].xyz / lspaceout[i].w;
    pcoord = pcoord * 0.5 + 0.5;
    if(pcoord.z > 1.0)
        return 0.0;
    float current = LinearizeDepth(pcoord.z);
    float shadow = 0.0;
    vec2 pixelsize = 1.0 / textureSize(shadows[i].shadowmap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
		    float pcf = LinearizeDepth(texture(shadows[i].shadowmap, pcoord.xy + vec2(x, y) * pixelsize).x);
		    shadow += float(current > pcf);
		}
	}
	shadow /= 9.0;
    return shadow;
}

float GGX(float ndoth, float rough)
{
    float dn = pi * pow(pow(ndoth, 2) * (pow(rough, 4) - 1.0) + 1.0, 2);
    
    return pow(rough, 4) / dn;
}

float GeometrySchlickGGX(float ndotv, float rough)
{
    float k = pow(rough + 1.0, 2) / 8.0;
    float dn = ndotv * (1.0 - k) + k;
	
    return ndotv / dn;
}

float GeometrySmith(float ndotv, float ndotl, float rough)
{
    float ggx1  = GeometrySchlickGGX(ndotv, rough);
    float ggx2  = GeometrySchlickGGX(ndotl, rough);
	
    return ggx1 * ggx2;
}

vec3 FresnelShlick(float cosTh, vec3 f0, float rough)
{
    return f0 + (max(vec3(1.0 - rough), f0) - f0) * pow(1.0 - cosTh, 5.0);
}

vec3 CalcLight(Light light, vec3 norm)
{
    float theta = dot(normalize(light.position - mpos), normalize(-light.direction));
    float intensity = 1.0;
    if(light.cutoff != 1.0)
        intensity = clamp((theta - light.outerCutoff) / (light.cutoff - light.outerCutoff), 0.0, 1.0);
    if(theta < light.cutoff && intensity <= 0.0) return vec3(0.0);

    vec3 v = normalize(camposout - mpos);

    float rough = (nroughness < 0.0 ? texture(roughness, coord).x : nroughness);
    float metal = (nmetalness < 0.0 ? texture(metalness, coord).x : nmetalness);
    vec3 albedo = (nalbedo.x < 0.0 ? texture(albedo, coord).xyz : nalbedo);

    vec3 f0 = mix(vec3(0.04), albedo, vec3(metal));

    vec3 l = (light.cutoff == 1.0 ? normalize(light.position - mpos) : normalize(-light.direction));
    vec3 h = normalize(v + l);

    float attenuation/* = 1.0 / (light.attenuation.x + light.attenuation.y * length(l) + light.attenuation.z * pow(length(l), 2))*/;

    float ndoth = max(dot(norm, h), 0.0);
    float ndotv = max(dot(norm, v), 0.0);
    float ndotl = max(dot(norm, l), 0.0);

    attenuation = 1.0 / pow(length(l), 2);

    vec3 rad = light.color * attenuation * intensity;

    float ndf = GGX(ndoth, rough);
    float g = GeometrySmith(ndotv, ndotl, rough);
    vec3 f = FresnelShlick(max(dot(h, v), 0.0), f0, rough);

    vec3 kspc = f;
    vec3 kdif = (vec3(1.0) - kspc) * (1.0 - metal);
    vec3 amb = (kdif * albedo) * (nao ? texture(ao, coord).xyz : vec3(1.0));

    vec3 nm = ndf * g * f;
    float dn = 4.0 * ndotv * ndotl;
    vec3 spc = (nm / max(dn, 0.001));
    
    vec3 lo = (amb / pi + spc) * rad * ndotl;

    return lo;
}

void main()
{
    vec3 norm;
    if(nnormalmap) norm = tbn * normalize(texture(normalmap, coord).xyz * 2.0 - 1.0);
    else norm = normalize(mnormal);

    vec3 emission = (nemission.x < 0.0 ? texture(emission, coord).xyz : nemission.xyz);

    /*vec3 reflected = reflect(normalize(mpos - camposout), normalize(norm));
    vec3 reflection = textureCube(cubemap, reflected).xyz;*/

    vec3 total = vec3(0.0);
    float shadow = 0.0;
    int i = 0;
    while(lights[i].isactive)
    {
        total += CalcLight(lights[i], norm);
        i++;
    }
    for(i = 0; i < maxshadows && shadows[i].isactive; i++)
	    shadow += CalcShadow(i);
    color = vec4((emission + total) * (1.0 - shadow + 0.1), (nopacity < 0.0 ? texture(opacity, coord).x : abs(nopacity)));
    //color = vec4(vec3(1.0 - shadow), 1.0);
}
