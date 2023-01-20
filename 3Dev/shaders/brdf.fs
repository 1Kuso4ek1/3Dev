#version 330
precision mediump float;

const float pi = 3.14159265;
uniform uint samples = 1024u;

in vec2 coord;

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

vec2 Hammersley(uint i, uint n)
{
    return vec2(float(i) / float(n), RadicalInverse(i));
}  

vec3 ImportanceSample(vec2 xi, vec3 n, float roughness)
{
    float phi = 2.0 * pi * xi.x;
    float cosTh = sqrt((1.0 - xi.y) / (1.0 + (pow(roughness, 4) - 1.0) * xi.y));
    float sinTh = sqrt(1.0 - pow(cosTh, 2));
	
    vec3 h = vec3(cos(phi) * sinTh, sin(phi) * sinTh, cosTh);
	
    vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, n));
    vec3 bitangent = cross(n, tangent);
	
    vec3 vec = tangent * h.x + bitangent * h.y + n * h.z;

    return normalize(vec);
}

float GeometrySchlick(float ndotv, float roughness)
{
    float k = pow(roughness, 2.0) / 2.0;

    return ndotv / ndotv * (1.0 - k) + k;
}

float GeometrySmith(float ndotv, float ndotl, float roughness)
{
    float ggx1  = GeometrySchlick(ndotv, roughness);
    float ggx2  = GeometrySchlick(ndotl, roughness);
	
    return ggx1 * ggx2;
}

vec2 BRDF(float ndotv, float roughness)
{
    vec3 v = vec3(sqrt(1.0 - pow(ndotv, 2.0)), 0.0, ndotv);

    vec2 ab = vec2(0.0);

    vec3 n = vec3(0.0, 0.0, 1.0);

    for(uint i = 0u; i < samples; i++)
    {
        vec2 xi = Hammersley(i, samples);
        vec3 h  = ImportanceSample(xi, n, roughness);
        vec3 l  = normalize(2.0 * dot(v, h) * h - v);

        float ndotl = max(l.z, 0.0);
        float ndoth = max(h.z, 0.0);
        float vdoth = max(dot(v, h), 0.0);

        if(ndotl > 0.0)
        {
            float g = GeometrySmith(max(dot(n, v), 0.0), max(dot(n, l), 0.0), roughness);
            float g1 = (g * vdoth) / (ndoth * ndotv);
            float fc = pow(1.0 - vdoth, 5.0);

            ab.x += (1.0 - fc) * g1;
            ab.y += fc * g1;
        }
    }
    ab /= float(samples);
    return ab;
}

void main() 
{
    color = vec4(BRDF(coord.x, coord.y), 0.0, 1.0);
}
