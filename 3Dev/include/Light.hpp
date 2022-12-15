#pragma once
#include "Utils.hpp"
#include "Shader.hpp"

/*
 * This class is used to create, modify and use a light.
 */
class Light
{
public:
	/*
	 * Basic constructor
	 * @param color color of the light
	 * @param position position of the light
	 * @param castsShadows if true, this light will cast shadows
	 */
	Light(rp3d::Vector3 color, rp3d::Vector3 position, bool castShadows = false);
	
	// @param color color of the light
	void SetColor(rp3d::Vector3 color);

	// @param position position of the light
	void SetPosition(rp3d::Vector3 position);

	// @param direction direction of the light
	void SetDirection(rp3d::Vector3 direction);

	/*
	 * Set light attenuation, calculated with formula "1.0 / (x + y * dist + z * dist * dist)"
	 * where: 
	 * x - constant attenuation
	 * y - linear attenuation
	 * z - quadratic attenuation
	 */
	void SetAttenuation(float constant, float linear, float quadratic);

	// @param cutoff new light cutoff value
	void SetCutoff(float cutoff);

	// @param outerCutoff new light outerCutoff value
	void SetOuterCutoff(float outerCutoff);
	
	/*
	 * Used to update the shader (used only in Model and Shape classes)
	 * @param shader pointer to the shader
	 * @param lightnum light index in the array
	 */
	void Update(Shader* shader, int lightnum);

	bool IsCastingShadows();

	// @return color of the light
	rp3d::Vector3 GetColor();

	// @return position of the light
	rp3d::Vector3 GetPosition();

	// @return direction of the light
	rp3d::Vector3 GetDirection();

	// @return vector that contains (constant, linear, quadratic)
	rp3d::Vector3 GetAttenuation();

	// @return light cutoff
	float GetCutoff();

	// @return light outerCutoff
	float GetOuterCutoff();

private:
	rp3d::Vector3 position, direction, color;

	bool castShadows = false;

	float constant = 1.0, linear = 0.0, quadratic = 0.0;
	float cutoff = 360.0, outerCutoff = 0.0;
};

