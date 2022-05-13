#version 330

const float maxTreshold = 0.5;
const float mulReduce = 1.0 / 8.0;
const float minReduce = 1.0 / 128.0;
const float maxSpan = 8.0;

in vec2 coord;

uniform vec2 pixelsize;
uniform sampler2D frame;
uniform float exposure = 1.0;
uniform bool fxaa = true;

out vec4 color;

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

void main()
{
    color = texture(frame, coord);
    if(fxaa) color = FXAA();
    
    color.rgb = color.rgb / (color.rgb + vec3(1.0));
    color.rgb *= exposure;
    
    float lum = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    float mlum = (lum * (1.0 + lum)) / (1.0 + lum);

    color.rgb = (mlum / lum) * color.rgb;
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
}
