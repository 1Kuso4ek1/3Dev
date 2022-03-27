#pragma once
#include "3Dev.h"
#include "Shader.h"

/*
 * This class is used to create, modify and use a light.
 */
class Light
{
public:
	/*
	 * Basic constructor
	 * @param ambient ambient color of the light
	 * @param diffuse diffuse color of the light
	 * @param specular specular color of the light
	 * @param position position of the light
	 */
	Light(rp3d::Vector3 ambient, rp3d::Vector3 diffuse, rp3d::Vector3 specular, rp3d::Vector3 position);
	
	// @param ambient ambient color of the light
	void SetAmbient(rp3d::Vector3 ambient);

	// @param diffuse diffuse color of the light
	void SetDiffuse(rp3d::Vector3 diffuse);

	// @param specular specular color of the light
	void SetSpecular(rp3d::Vector3 specular);

	// @param position position of the light
	void SetPosition(rp3d::Vector3 position);

	/* 
	 * Set light attenuation, calculated with formula "1.0 / (x + y * dist + z * dist * dist)"
	 * where: 
	 * x - constant attenuation
	 * y - linear attenuation
	 * z - quadratic attenuation
	 */
	void SetAttenuation(float constant, float linear, float quadratic);
	
	/*
	 * Used to update the shader (used only in Model and Shape classes)
	 * @param shader pointer to the shader
	 * @param lightnum light index in the array
	 */
	void Update(Shader* shader, int lightnum);

	// @return ambient color of the light
	rp3d::Vector3 GetAmbient();

	// @return diffuse color of the light
	rp3d::Vector3 GetDiffuse();

	// @return specular color of the light
	rp3d::Vector3 GetSpecular();

	// @return position of the light
	rp3d::Vector3 GetPosition();

	// @return vector that contains (constant, linear, quadratic)
	rp3d::Vector3 GetAttenuation();

private:
	rp3d::Vector3 position, ambient, diffuse, specular;
	float constant = 1.0, linear = 0.0, quadratic = 0.0;
};

