#include "Light.h"

Light::Light(GLenum lightNum) : lightNum(lightNum) {
	glEnable(lightNum);
}

void Light::SetParameters(float parameters[4], GLenum type)
{
	glLightfv(lightNum, type, parameters);
}

