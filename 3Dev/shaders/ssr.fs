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

vec3 FresnelSchlick(float cosTh, vec3 f0, float rough)
{
    return f0 + (max(vec3(1.0 - rough), f0) - f0) * pow(1.0 - cosTh, 5.0);
}

vec2 UV(vec3 pos)
{
	vec4 p = projection * vec4(pos, 1.0);
	return (p.xy / p.w) * 0.5 + 0.5;
}

vec3 SSR(vec3 pos, vec3 reflected)
{
    vec3 rayStep = 0.1 * reflected;
	vec3 marchingPos = pos + rayStep;
    float delta = 0.0;
	
	for(int i = 0; i < 50; i++)
    {
		uv = UV(marchingPos);
        float z = abs(texture(gposition, uv).z);
        if(z > 100.0) break;

		delta = abs(marchingPos.z) - z;
        falloff = 1.0 - abs(distance(pos, marchingPos) / 7);
        
        if(clamp(falloff, 0.0, 1.0) <= 0.0) break;
        if(abs(delta) < 0.01) return texture(gcolor, uv).rgb;

        rayStep = rayStep * (1.0 - 0.1 * max(sign(delta), 0.0));
        marchingPos += rayStep * (-sign(delta));
    }

    return vec3(0.0);
}

void main()
{
    vec4 pos = texture(gposition, coord);
    vec3 reflected = normalize(reflect(pos.xyz, normalize(texture(gnormal, coord).xyz)));
    if(pos.w <= 0.0) discard;

    vec4 combined = texture(gcombined, coord);

    vec3 ssr = SSR(pos.xyz, reflected);
    vec3 f = FresnelSchlick(max(dot(pos.xyz, normalize(texture(gnormal, coord).xyz)), 0.0), mix(vec3(0.04), texture(gcolor, coord).xyz, combined.x), combined.y);
    vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - uv));
    float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

    float reflectionCoef = pow(1.0 - combined.y, 3.0) * screenEdgefactor * -reflected.z;

    color = vec4(ssr * f * clamp(reflectionCoef, 0.0, 0.9), clamp(falloff, 0.0, 1.0));
}
