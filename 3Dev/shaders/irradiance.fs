#version 450 core

const float pi = 3.14159265;

uniform samplerCube cubemap;

in vec3 coord;

out vec4 color;

void main()
{
    vec3 norm = normalize(coord);
    vec3 irradiance = vec3(0.0);  

    vec3 right = cross(vec3(0.0, 1.0, 0.0), norm);
    vec3 up = cross(norm, right);

    float delta = 0.025;
    float samples = 0.0; 
    for(float phi = 0.0; phi < 2.0 * pi; phi += delta)
    {
        for(float theta = 0.0; theta < 0.5 * pi; theta += delta)
        {
            vec3 s = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            vec3 vec = s.x * right + s.y * up + s.z * norm; 

            irradiance += texture(cubemap, vec).rgb * cos(theta) * sin(theta);
            samples++;
        }
    }
    color = vec4(pi * irradiance * (1.0 / samples), 1.0);
}
