#version 330
layout (location = 0) in vec3 position;
layout (location = 3) in vec4 ids;
layout (location = 4) in vec4 weights;

uniform mat4 light;
uniform mat4 model;

uniform mat4 pose[64];
uniform bool bones;
uniform mat4 transformation;

void main()
{
	vec4 pos = vec4(position, 1.0);
	mat4 transform = transformation;
	
    if(bones)
    {
        transform = mat4(0.0);
        transform += pose[int(ids.x)] * weights.x;
        transform += pose[int(ids.y)] * weights.y;
        transform += pose[int(ids.z)] * weights.z;
        transform += pose[int(ids.w)] * weights.w;
    }
    pos = transform * pos;
    
    gl_Position = light * model * pos;
}
