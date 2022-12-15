#include "Light.hpp"

Light::Light(rp3d::Vector3 color, rp3d::Vector3 position, bool castsShadows) 
			: castShadows(castShadows), color(color), position(position) {}

void Light::SetColor(rp3d::Vector3 color)
{
	this->color = color;
}

void Light::SetPosition(rp3d::Vector3 position)
{
	this->position = position;
}

void Light::SetDirection(rp3d::Vector3 direction)
{
	this->direction = direction;
}

void Light::SetAttenuation(float constant, float linear, float quadratic)
{
	this->constant = constant;
	this->linear = linear;
	this->quadratic = quadratic;
}

void Light::SetCutoff(float cutoff)
{
	this->cutoff = cutoff;
}

void Light::SetOuterCutoff(float outerCutoff)
{
	this->outerCutoff = outerCutoff;
}

void Light::Update(Shader* shader, int lightnum) 
{
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].color", color.x, color.y, color.z);
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].position", position.x, position.y, position.z);
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].direction", direction.x, direction.y, direction.z);
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].attenuation", constant, linear, quadratic);
	shader->SetUniform1f("lights[" + std::to_string(lightnum) + "].cutoff", glm::cos(glm::radians(cutoff)));
	shader->SetUniform1f("lights[" + std::to_string(lightnum) + "].outerCutoff", glm::cos(glm::radians(outerCutoff)));
	shader->SetUniform1i("lights[" + std::to_string(lightnum) + "].isactive", 1);
}

bool Light::IsCastingShadows()
{
	return castShadows;
}

rp3d::Vector3 Light::GetColor() 
{
	return color;
}

rp3d::Vector3 Light::GetPosition() 
{
	return position;
}

rp3d::Vector3 Light::GetDirection()
{
	return direction;
}

rp3d::Vector3 Light::GetAttenuation()
{
	return rp3d::Vector3(constant, linear, quadratic);
}

float Light::GetCutoff()
{
	return cutoff;
}

float Light::GetOuterCutoff()
{
	return outerCutoff;
}
