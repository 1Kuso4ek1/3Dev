#version 330
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 ids;
layout (location = 4) in vec4 weights;

uniform mat4 pose[64];
uniform bool bones;
uniform mat4 transformation;

uniform vec3 campos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 coord;
out vec3 camposout;
out vec3 mnormal;
out vec3 mpos;
out mat3 tbn;

void main()
{
    vec4 pos = vec4(position, 1.0);
    mat4 transform = mat4(1.0);

    if(bones)
    {
        transform = mat4(0.0);
        vec4 w = normalize(weights);
        transform += pose[int(ids.x)] * w.x;
        transform += pose[int(ids.y)] * w.y;
        transform += pose[int(ids.z)] * w.z;
        transform += pose[int(ids.w)] * w.w;
        pos = transform * (inverse(transformation) * pos);
        pos = transformation * pos;
    }

    mpos = (model * pos).xyz;
    mnormal = normalize(mat3(model * transform) * normal);
    coord = uv;
    camposout = campos;

    vec3 tangent = cross(mnormal, vec3(0.5, 0.5, 0.5));
    vec3 t = normalize(mat3(model * transform) * tangent);
    vec3 n = mnormal;
    vec3 b = cross(n, t);
    tbn = mat3(t, b, n);

    gl_Position = (projection * view * model) * pos;
}
