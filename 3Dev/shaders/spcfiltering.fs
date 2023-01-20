#version 330
precision mediump float;

const float pi = 3.14159265;

uniform samplerCube cubemap;
uniform uint samples = 1024u;
uniform float roughness = 0.05;

in vec3 coord;

out vec4 color;

float RadicalInverse(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

float GGX(float ndoth)
{
    float dn = pi * pow(pow(ndoth, 2.0) * (pow(roughness, 4.0) - 1.0) + 1.0, 2.0);
    
    return pow(roughness, 4.0) / dn;
}

vec2 Hammersley(uint i, uint n)
{
    return vec2(float(i) / float(n), RadicalInverse(i));
}  

vec3 ImportanceSample(vec2 xi, vec3 n)
{
    float phi = 2.0 * pi * xi.x;
    float cosTh = sqrt((1.0 - xi.y) / (1.0 + (pow(roughness, 4.0) - 1.0) * xi.y));
    float sinTh = sqrt(1.0 - pow(cosTh, 2.0));
	
    vec3 h = vec3(cos(phi) * sinTh, sin(phi) * sinTh, cosTh);
	
    vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, n));
    vec3 bitangent = cross(n, tangent);
	
    vec3 vec = tangent * h.x + bitangent * h.y + n * h.z;

    return normalize(vec);
}

void main()
{
    color = vec4(0.0, 0.0, 0.0, 1.0);

    vec3 n, r, v;
    v = r = n = normalize(coord);

    float w = 0.0;

    float size = float(textureSize(cubemap, 0));

    for(uint i = 0u; i < samples; i++)
    {
        vec2 xi = Hammersley(i, samples);
        vec3 h  = ImportanceSample(xi, n);
        vec3 l  = normalize(2.0 * dot(v, h) * h - v);

        float ggx = GGX(dot(n, h));
        float pdf = (ggx * dot(n, h) / (4.0 * dot(h, v))) + 0.0001;

        float t = 4.0 * pi / (6.0 * pow(size, 2.0));
        float s = 1.0 / (float(samples) * pdf + 0.0001);

        float level = (roughness == 0.0 ? 0.0 : 0.5 * log2(s / t)); 
        
        float ndotl = max(dot(n, l), 0.0);
        if(ndotl > 0.0)
        {
            color += textureLod(cubemap, l, level) * ndotl;
            w += ndotl;
        }
    }
    color /= w;
}
