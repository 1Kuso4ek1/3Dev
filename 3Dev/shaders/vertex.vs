#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

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
    mpos = (model * vec4(position, 1.0)).xyz;
    mnormal = normalize(mat3(model) * normal);
    coord = uv;
    camposout = campos;

    vec3 tangent = cross(mnormal, vec3(0.5, 0.5, 0.5));
    vec3 t = normalize(mat3(model) * tangent);
    vec3 n = mnormal;
    vec3 b = cross(n, t);
    tbn = mat3(t, b, n);

    gl_Position = (projection * view * model) * vec4(position, 1.0);
}