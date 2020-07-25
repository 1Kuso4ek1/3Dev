#include "Light.h"

Light::Light(GLenum lightNum) : lightNum(lightNum) {
	glEnable(lightNum);
}

void Light::SetParameters(float parameters[], GLenum type)
{
	glLightfv(lightNum, type, parameters);
}

