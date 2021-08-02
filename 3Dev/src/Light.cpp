#include "Light.h"

Light::Light(GLenum lightNum, float x, float y, float z, std::string ID) : lightNum(lightNum), ID(ID) 
{
	Enable();
	SetPosition(x, y, z);
}

Light::Light(GLenum lightNum, float x, float y, float z) : lightNum(lightNum) 
{
	Enable();
	SetPosition(x, y, z);
}

Light::~Light()
{
	glDisable(lightNum);
}

void Light::SetParameters(std::vector<float> parameters, GLenum type)
{
	glLightfv(lightNum, type, &parameters[0]);
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
	SetParameters({ position.x, position.y, position.z, 1 }, GL_POSITION);
}

void Light::Enable()
{
	glEnable(lightNum);
}

void Light::Disable()
{
	glDisable(lightNum);
}

void Light::EnableLighting()
{
	glEnable(GL_LIGHTING);
}

void Light::DisableLighting()
{
	glDisable(GL_LIGHTING);
}

std::vector<float> Light::GetParameters(GLenum type)
{
	float var[] = { 0, 0, 0, 0 };
	glGetLightfv(lightNum, type, var);
	return std::vector<float>(var, var + sizeof(var) / sizeof(float));
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
