#version 450 core

uniform vec2 pixelsize;
uniform sampler2D gposition;
uniform sampler2D gcolor;
uniform sampler2D gnormal;
uniform sampler2D gcombined;

uniform mat4 projection;

uniform int maxSteps = 100;
uniform int maxBinarySearchSteps = 20;

uniform float rayStep = 0.01;

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
    for(int i = 0; i < maxBinarySearchSteps; i++)
    {
        uv = UV(pos);
 
        float depth = texture(gposition, uv).z;
 
        float delta = pos.z - depth;
        if(abs(delta) < 0.1) return;

        dir *= 0.5;
        pos += dir * sign(delta);
    }
 
    uv = vec2(-1.0);
}

vec3 SSR(vec3 dir, vec3 pos)
{
    dir *= rayStep;
 
    for(int i = 0; i < maxSteps; i++)
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
                if(uv != vec2(-1.0))
                    return texture(gcolor, uv).rgb;
                else break;
            }
        }
    }

    return vec3(0.0);
}

float rand(vec2 v)
{
    return fract(sin(dot(v, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 hash(vec3 a)
{
    a = fract(a * 0.8);
    a += dot(a, a.yxz + 19.19);
    return fract((a.xxy + a.yxx) * a.zyx) * rand(a.yz) * 2.0;
}

void main()
{
    vec3 pos = texture(gposition, coord).xyz;
    vec3 reflected = normalize(reflect(pos, normalize(texture(gnormal, coord).xyz)));

    vec4 combined = texture(gcombined, coord);

    if(combined.x == 0.0 && combined.y >= 0.9)
        discard;

    vec3 jitt = mix(vec3(0.0), hash((pos).xyz), combined.y);
    vec3 ssr = SSR((jitt + reflected * max(0.1, -pos.z)), pos);

    vec2 d = smoothstep(0.3, 0.8, abs(vec2(0.5, 0.5) - uv));
    float screenEdge = clamp(1.0 - (d.x + d.y), 0.0, 1.0);

    color = vec4(ssr, clamp((((1.0 - combined.y) + combined.x) / 2.0), 0.1, 1.0) * screenEdge);
}
