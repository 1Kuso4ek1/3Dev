#include "Light.h"

Light::Light(rp3d::Vector3 ambient, rp3d::Vector3 diffuse, rp3d::Vector3 specular, rp3d::Vector3 position) : ambient(ambient), diffuse(diffuse), specular(specular), position(position) {}

void Light::SetAmbient(rp3d::Vector3 ambient)
{
	this->ambient = ambient;
}

void Light::SetDiffuse(rp3d::Vector3 diffuse)
{
	this->diffuse = diffuse;
}

void Light::SetSpecular(rp3d::Vector3 specular)
{
	this->specular = specular;
}

void Light::SetPosition(rp3d::Vector3 position)
{
	this->position = position;
}

void Light::SetAttenuation(float constant, float linear, float quadratic)
{
	this->constant = constant;
	this->linear = linear;
	this->quadratic = quadratic;
}

void Light::Update(Shader* shader, int lightnum) 
{
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].ambient", ambient.x, ambient.y, ambient.z);
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].diffuse", diffuse.x, diffuse.y, diffuse.z);
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].specular", specular.x, specular.y, specular.z);
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].position", position.x, position.y, position.z);
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].attenuation", constant, linear, quadratic);
	shader->SetUniform1i("lights[" + std::to_string(lightnum) + "].isactive", 1);
}

rp3d::Vector3 Light::GetAmbient() 
{
	return ambient;
}

rp3d::Vector3 Light::GetDiffuse() 
{
	return diffuse;
}

rp3d::Vector3 Light::GetSpecular() 
{
	return specular;
}

rp3d::Vector3 Light::GetPosition() 
{
	return position;
}

rp3d::Vector3 Light::GetAttenuation()
{
	return rp3d::Vector3(constant, linear, quadratic);
}
