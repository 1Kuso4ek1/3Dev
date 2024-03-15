#version 450 core

const float maxTreshold = 0.5;
const float mulReduce = 1.0 / 8.0;
const float minReduce = 1.0 / 128.0;
const float maxSpan = 8.0;

in vec2 coord;

uniform vec2 pixelsize;
uniform sampler2D frame;
uniform sampler2D bloom;
uniform sampler2D ssr;
uniform sampler2D ssgi;
uniform sampler2D fog;
uniform sampler2D gcombined;
uniform sampler2D decalsCombined;
uniform sampler2D galbedo;
uniform sampler2D frameDepth;
uniform sampler2D transparencyDepth;

uniform float exposure = 1.0;
uniform float bloomStrength = 0.3;
uniform float dofMinDistance = 1.0;
uniform float dofMaxDistance = 1.0;
uniform float dofFocusDistance = 1.0;
uniform float fogIntensity = 1.0;

uniform bool fxaa = true;
uniform bool fogEnabled;
uniform bool ssrEnabled;
uniform bool rawColor;
uniform bool transparentBuffer;

out vec4 color;

mat3 acesIn = mat3(0.59719, 0.07600, 0.02840,
                   0.35458, 0.90834, 0.13383,
                   0.04823, 0.01566, 0.83777);

mat3 acesOut = mat3(1.60475, -0.10208, -0.00327,
                    -0.53108,  1.10813, -0.07276,
                    -0.07367, -0.00605, 1.07602);

vec4 FXAA()
{
    float color0 = dot(color.xyz, vec3(0.299, 0.587, 0.114));
    float color1 = dot(textureOffset(frame, coord, ivec2(-1, 1)).xyz, vec3(0.299, 0.587, 0.114));
    float color2 = dot(textureOffset(frame, coord, ivec2(1, 1)).xyz, vec3(0.299, 0.587, 0.114));
    float color3 = dot(textureOffset(frame, coord, ivec2(-1, -1)).xyz, vec3(0.299, 0.587, 0.114));
    float color4 = dot(textureOffset(frame, coord, ivec2(1, -1)).xyz, vec3(0.299, 0.587, 0.114));

    float cmin = min(color0, min(min(color1, color2), min(color3, color4)));
    float cmax = max(color0, max(max(color1, color2), max(color3, color4)));

    if(cmax - cmin <= cmax * maxTreshold)
        return color;

	vec2 sampleDir = vec2(-((color1 + color2) - (color3 + color4)),
                           ((color1 + color3) - (color2 + color4)));

    float dirReduce = max((color1 + color2 + color3 + color4) * 0.25 * mulReduce, minReduce);

	float factor = 1.0 / (min(abs(sampleDir.x), abs(sampleDir.y)) + dirReduce);

    sampleDir = clamp(sampleDir * factor, vec2(-maxSpan), vec2(maxSpan)) * pixelsize;

	vec3 samplePos1 = texture(frame, coord + sampleDir * (1.0 / 3.0 - 0.5)).xyz;
	vec3 samplePos2 = texture(frame, coord + sampleDir * (2.0 / 3.0 - 0.5)).xyz;

	vec3 tab2 = (samplePos1 + samplePos2) * 0.5;

	vec3 samplePosOut1 = texture(frame, coord + sampleDir * (0.0 - 0.5)).xyz;
	vec3 samplePosOut2 = texture(frame, coord + sampleDir * (1.0 - 0.5)).xyz;

	vec3 tab4 = (samplePosOut1 + samplePosOut2) * 0.25 + tab2 * 0.5;

	float lumTab4 = dot(tab4, vec3(0.299, 0.587, 0.114));

	if(lumTab4 < cmin || lumTab4 > cmax)
		return vec4(tab2, color.w);
	else return vec4(tab4, color.w);
}

vec3 ACES()
{
    vec3 v = acesIn * color.rgb * exposure;
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return acesOut * (a / b);
}

void main()
{
    if(transparentBuffer)
    {
        if(texture(frameDepth, coord).x < texture(transparencyDepth, coord).x - 0.0000001)
        {
            color = vec4(0.0, 0.0, 0.0, 0.0);
            return;
        }
    }

    color = texture(frame, coord);

    if(rawColor)
    {
        color.rgb = max(color.rgb, vec3(0.00001));
        if(ssrEnabled && !transparentBuffer)
        {
            vec4 combined = texture(gcombined, coord);
            float lod = 8.0 * pow(combined.y, 2.0);

            vec3 f0 = mix(vec3(0.04), texture(galbedo, coord).rgb, combined.x);

            vec4 ssr = texture(ssr, coord + sqrt(lod) * pixelsize, lod);
            color.rgb += f0 * ssr.rgb;
        }

        vec4 fog = texture(fog, coord);
        
        if(!transparentBuffer && fogEnabled)
            color.rgb = mix(color.rgb, fog.rgb, fog.a * fogIntensity);

        return;
    }

    if(fxaa) color = FXAA();

    float depth = pow(transparentBuffer ? texture(transparencyDepth, coord).x : texture(frameDepth, coord).x, 300.0);
    float dof = smoothstep(dofMinDistance, dofMaxDistance, abs(depth - dofFocusDistance));

    vec4 combined = texture(decalsCombined, coord);
    if(length(combined) == 0.0)
        combined = texture(gcombined, coord);

    if(ssrEnabled && !transparentBuffer)
    {
        float lod = 8.0 * pow(combined.y, 2.0);

        vec3 f0 = mix(vec3(0.04), texture(galbedo, coord).rgb, combined.x);

        vec4 ssr = texture(ssr, coord + sqrt(lod) * pixelsize, lod);
        color.rgb += f0 * ssr.rgb;
    }

    vec3 ssgi = texture(ssgi, coord).rgb;

    color.rgb += mix(texture(galbedo, coord).rgb, color.rgb, combined.x) * ssgi;

    vec4 fog = texture(fog, coord);

    if(!transparentBuffer && fogEnabled)
        color.rgb = mix(color.rgb, mix(fog.rgb, ssgi, 0.5), fog.a * fogIntensity);

    color.rgb = mix(color.rgb, texture(bloom, coord).rgb, clamp(bloomStrength + dof, 0.0, 1.0));

    color.rgb = ACES();
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
}
