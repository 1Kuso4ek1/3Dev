#version 450 core

uniform vec2 pixelsize;
uniform sampler2D gposition;
uniform sampler2D gcolor;
uniform sampler2D gnormal;
uniform sampler2D gcombined;

uniform mat4 view;
uniform mat4 projection;

uniform vec3 camPos;

in vec2 coord;

out vec4 color;

float falloff;

vec2 uv;

vec2 UV(vec3 pos)
{
	vec4 p = projection * vec4(pos, 1.0);
	return (p.xy / p.w) * 0.5 + 0.5;
}

void BinarySearch(vec3 dir, vec3 pos)
{
    for(int i = 0; i < 5; i++)
    {
        uv = UV(pos);
 
        float depth = texture(gposition, uv).z;
 
        float delta = pos.z - depth;

        dir *= 0.5;
        pos += dir * sign(delta);
    }
 
    uv = UV(pos);
}

vec3 SSR(vec3 dir, vec3 pos)
{
    dir *= 0.1;
 
    for(int i = 0; i < 30; i++)
    {
        pos += dir;
 
        uv = UV(pos);
 
        float depth = texture(gposition, uv).z;
 
        float delta = pos.z - depth;

        if((dir.z - delta) < 1.2)
        {
            if(delta <= 0.0)
            {
                BinarySearch(dir, pos);
                return texture(gcolor, uv).rgb;
            }
        }
    }

    return vec3(0.0);
}

vec3 hash(vec3 a)
{
    a = fract(a * 0.8);
    a += dot(a, a.yxz + 19.19);
    return fract((a.xxy + a.yxx) * a.zyx);
}

vec3 FresnelSchlick(float cosTh, vec3 f0, float rough)
{
    return f0 + (max(vec3(1.0 - rough), f0) - f0) * pow(1.0 - cosTh, 5.0);
}

void main()
{
    vec4 pos = texture(gposition, coord);
    vec3 reflected = normalize(reflect(pos.xyz, normalize(texture(gnormal, coord).xyz)));

    vec4 combined = texture(gcombined, coord);

    vec3 jitt = mix(vec3(0.0), hash((pos).xyz), combined.y);
    vec3 ssr = SSR((jitt + reflected * max(0.1, -pos.z)), pos.xyz);

    vec2 d = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - uv));
    float screenEdge = clamp(1.0 - (d.x + d.y), 0.0, 1.0);

    color = vec4(ssr * screenEdge * 0.6, clamp(((1.0 - combined.y) / 1.5) + combined.x, 0.0, 1.0));
}
