#include "Light.hpp"

Light::Light(Light* light)
{
	color = light->color;
	transform = light->transform;
	constant = light->constant;
	linear = light->linear;
	quadratic = light->quadratic;
	cutoff = light->cutoff;
	outerCutoff = light->outerCutoff;
	castShadows = light->castShadows;
	perspectiveShadows = light->perspectiveShadows;
	lightSpaceMatrix = light->lightSpaceMatrix;
}

Light::Light(const rp3d::Vector3& color, const rp3d::Vector3& position, bool castShadows) 
			: castShadows(castShadows), color(color)
{
	transform.setPosition(position);
	if(castShadows)
		CalcLightSpaceMatrix();
}

void Light::SetColor(const rp3d::Vector3& color)
{
	this->color = color;
}

void Light::SetPosition(const rp3d::Vector3& position)
{
	transform.setPosition(position);
}

void Light::SetOrientation(const rp3d::Quaternion& orientation)
{
	transform.setOrientation(orientation);
}

void Light::SetTransform(const rp3d::Transform& transform)
{
	this->transform = transform;
}

void Light::Move(const rp3d::Vector3& vec)
{
	transform.setPosition(transform.getPosition() + vec);
}

void Light::Rotate(const rp3d::Quaternion& quat)
{
	transform.setOrientation(quat * transform.getOrientation());
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

void Light::CalcLightSpaceMatrix()
{
	glm::mat4 projection;

	auto tr = Node::GetFinalTransform(this) * transform;
	auto pos = tr.getPosition();

	if(perspectiveShadows)
		projection = glm::perspective(glm::radians(90.0), 1.0, 0.01, 1000.0);
	else 
		projection = glm::ortho(-200.0, 200.0, -200.0, 200.0, 0.01, 1000.0);
	
	glm::mat4 view = glm::lookAt(toglm(pos), toglm((tr.getOrientation() * rp3d::Vector3(0, 0, -1)) + pos), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = projection * view;
}

void Light::SetIsCastingShadows(bool castShadows)
{
	this->castShadows = castShadows;
}

void Light::SetIsCastingPerspectiveShadows(bool perspectiveShadows)
{
	this->perspectiveShadows = perspectiveShadows;
}

void Light::Update(Shader* shader, int lightnum) 
{
	auto tr = Node::GetFinalTransform(this) * transform;
	auto pos = tr.getPosition();

	auto direction = tr.getOrientation() * rp3d::Vector3(0, 0, -1);

	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].color", color.x, color.y, color.z);
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].position", pos.x, pos.y, pos.z);
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].direction", direction.x, direction.y, direction.z);
	shader->SetUniform3f("lights[" + std::to_string(lightnum) + "].attenuation", constant, linear, quadratic);
	shader->SetUniform1f("lights[" + std::to_string(lightnum) + "].cutoff", glm::cos(glm::radians(cutoff)));
	shader->SetUniform1f("lights[" + std::to_string(lightnum) + "].outerCutoff", glm::cos(glm::radians(outerCutoff)));
	shader->SetUniform1i("lights[" + std::to_string(lightnum) + "].isactive", 1);
	shader->SetUniform1i("lights[" + std::to_string(lightnum) + "].castShadows", castShadows);
}

bool Light::IsCastingShadows()
{
	return castShadows;
}

bool Light::IsCastingPerspectiveShadows()
{
	return perspectiveShadows;
}

glm::mat4 Light::GetLightSpaceMatrix()
{
	if(castShadows)
		CalcLightSpaceMatrix();
	return lightSpaceMatrix;
}

rp3d::Vector3 Light::GetColor() 
{
	return color;
}

rp3d::Vector3 Light::GetPosition(bool world) 
{
	if(world)
		return (Node::GetFinalTransform(this) * transform).getPosition();
	return transform.getPosition();
}

rp3d::Quaternion Light::GetOrientation()
{
	return transform.getOrientation();
}

rp3d::Vector3 Light::GetAttenuation()
{
	return rp3d::Vector3(constant, linear, quadratic);
}

rp3d::Transform Light::GetTransform()
{
	return transform;
}

float Light::GetCutoff()
{
	return cutoff;
}

float Light::GetOuterCutoff()
{
	return outerCutoff;
}

Json::Value Light::Serialize()
{
	Json::Value data;

	data["position"]["x"] = transform.getPosition().x;
	data["position"]["y"] = transform.getPosition().y;
	data["position"]["z"] = transform.getPosition().z;

	data["orientation"]["x"] = transform.getOrientation().x;
	data["orientation"]["y"] = transform.getOrientation().y;
	data["orientation"]["z"] = transform.getOrientation().z;
	data["orientation"]["w"] = transform.getOrientation().w;

	data["color"]["r"] = color.x;
	data["color"]["g"] = color.y;
	data["color"]["b"] = color.z;

	data["attenuation"]["constant"] = constant;
	data["attenuation"]["linear"] = linear;
	data["attenuation"]["quadratic"] = quadratic;

	data["cutoff"] = cutoff;
	data["outerCutoff"] = outerCutoff;

	data["castShadows"] = castShadows;
	data["perspectiveShadows"] = perspectiveShadows;

	return data;
}

void Light::Deserialize(Json::Value data)
{
	rp3d::Vector3 position;
	position.x = data["position"]["x"].asFloat();
	position.y = data["position"]["y"].asFloat();
	position.z = data["position"]["z"].asFloat();

	rp3d::Quaternion orientation;
	orientation.x = data["orientation"]["x"].asFloat();
	orientation.y = data["orientation"]["y"].asFloat();
	orientation.z = data["orientation"]["z"].asFloat();
	orientation.w = data["orientation"]["w"].asFloat();

	color.x = data["color"]["r"].asFloat();
	color.y = data["color"]["g"].asFloat();
	color.z = data["color"]["b"].asFloat();

	constant = data["attenuation"]["constant"].asFloat();
	linear = data["attenuation"]["linear"].asFloat();
	quadratic = data["attenuation"]["quadratic"].asFloat();

	cutoff = data["cutoff"].asFloat();
	outerCutoff = data["outerCutoff"].asFloat();

	castShadows = data["castShadows"].asBool();
	perspectiveShadows = data["perspectiveShadows"].asBool();

	if(castShadows)
		CalcLightSpaceMatrix();

	transform.setPosition(position);
	transform.setOrientation(orientation);
}
