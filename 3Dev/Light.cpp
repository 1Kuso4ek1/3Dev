#include "Light.h"

Light::Light(GLenum lightNum, float x, float y, float z) : lightNum(lightNum), x(x), y(y), z(z) {
	glEnable(lightNum);
}

void Light::SetParameters(float parameters[], GLenum type)
{
	glLightfv(lightNum, type, parameters);
}

void Light::SetPosition(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}
