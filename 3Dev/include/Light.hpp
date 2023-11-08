#pragma once
#include "Utils.hpp"
#include "Shader.hpp"
#include "Node.hpp"

class Light : public Node
{
public:
	Light(const rp3d::Vector3& color, const rp3d::Vector3& position, bool castShadows = false);
	
	void SetColor(const rp3d::Vector3& color);
	void SetPosition(const rp3d::Vector3& position);
	void SetOrientation(const rp3d::Quaternion& orientation);

	void Move(const rp3d::Vector3& vec);
	void Rotate(const rp3d::Quaternion& quat);

	/*
	 * Set light attenuation, calculated with formula "1.0 / (x + y * dist + z * dist * dist)"
	 * where: 
	 * x - constant attenuation
	 * y - linear attenuation
	 * z - quadratic attenuation
	 */
	void SetAttenuation(float constant, float linear, float quadratic);

	void SetCutoff(float cutoff);
	void SetOuterCutoff(float outerCutoff);

	void CalcLightSpaceMatrix();

	void SetIsCastingShadows(bool castShadows);
	void SetIsCastingPerspectiveShadows(bool perspectiveShadows);
	void Update(Shader* shader, int lightnum);

	bool IsCastingShadows();
	bool IsCastingPerspectiveShadows();

	glm::mat4 GetLightSpaceMatrix();

	rp3d::Vector3 GetColor();
	rp3d::Vector3 GetPosition(bool world = false);
	rp3d::Quaternion GetOrientation();
	rp3d::Vector3 GetAttenuation();

	rp3d::Transform GetTransform() override;

	float GetCutoff();
	float GetOuterCutoff();

	Json::Value Serialize();
	void Deserialize(Json::Value data);

private:
	glm::mat4 lightSpaceMatrix;

	rp3d::Vector3 color;
	rp3d::Transform transform;

	bool castShadows = false, perspectiveShadows = false;

	float constant = 1.0, linear = 0.0, quadratic = 0.0;
	float cutoff = 360.0, outerCutoff = 0.0;
};

