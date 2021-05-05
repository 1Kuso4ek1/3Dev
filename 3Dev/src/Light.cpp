#include "Light.h"

Light::Light(GLenum lightNum, float x, float y, float z, std::string ID) : lightNum(lightNum), ID(ID) {
	glEnable(lightNum);
	SetPosition(x, y, z);
}

Light::Light(GLenum lightNum, float x, float y, float z) : lightNum(lightNum) {
	glEnable(lightNum);
	SetPosition(x, y, z);
}

Light::~Light()
{
	glDisable(lightNum);
}

void Light::SetParameters(float parameters[], GLenum type)
{
	glLightfv(lightNum, type, parameters);
}

void Light::SetPosition(float x, float y, float z)
{
	position = sf::Vector3f(x, y, z);
}

void Light::SetID(std::string ID) 
{
	this->ID = ID;
}

void Light::AddPosition(float x, float y, float z)
{
	SetPosition(position.x + x, position.y + y, position.z + z);
}

void Light::Update() 
{
	float parameters[4] = { position.x, position.y, position.z, 1 };
	SetParameters(parameters, GL_POSITION);
}

void Light::GetParameters(GLenum type, float* var)
{
	glGetLightfv(lightNum, type, var);
}

GLenum Light::GetLightNum() 
{
	return lightNum;
}

sf::Vector3f Light::GetPosition() 
{
	return position;
}

std::string Light::GetID() {
	return ID;
}
