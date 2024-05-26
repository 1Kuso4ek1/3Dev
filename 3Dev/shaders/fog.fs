#version 450 core

uniform sampler2D gposition;
uniform sampler2D gemission;
uniform samplerCube irradiance;

uniform vec3 campos;
uniform vec3 camdir;

uniform float fogStart = 10.0;
uniform float fogEnd = 30.0;
uniform float fogHeight = 5.0;
uniform float fogSkyHeight = 100.0;

uniform mat4 invView;

in vec2 coord;

out vec4 color;

float LayeredFog(vec3 pos)
{
	vec3 camProj = campos; camProj.y = 0.0;
	vec3 worldProj = pos; worldProj.y = 0.0;
	
	float deltaD = length(camProj - worldProj) / fogEnd;
	float deltaY = 0.0;
	float density = 0.0;
	
	if(pos.y < fogHeight)
	{
		deltaY = (fogHeight - pos.y) / fogHeight;
		density = pow(deltaY, 2.0) * 0.5;
	}

	if(deltaY != 0.0 && fogEnd != fogStart)
		return clamp((sqrt(1.0 + ((deltaD / deltaY) * (deltaD / deltaY)))) * density * clamp((length(campos - pos) - fogStart) / (fogEnd - fogStart), 0.0, 1.0), 0.0, 1.0); 
	else return 0.0;
}

void main()
{
    vec3 viewPos = texture(gposition, coord).xyz;
    vec3 pos = (invView * vec4(viewPos, 1.0)).xyz;
	vec3 cubemapPos = normalize(pos);

    if(length(viewPos) == 0.0)
	{
        pos = vec3(1000000.0, fogSkyHeight, 1000000.0);
		cubemapPos = normalize(campos);
	}

    color = vec4(texture(irradiance, cubemapPos).xyz, LayeredFog(pos) * (1.0 - clamp(length(texture(gemission, coord).rgb), 0.0, 1.0)));
}
