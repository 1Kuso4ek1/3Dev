#pragma once
#include "3Dev.h"

class Light
{
public:
	Light(GLenum lightNum, float x, float y, float z, std::string ID);
	Light(GLenum lightNum, float x, float y, float z);

	~Light();
	
	void SetParameters(std::vector<float> parameters, GLenum type);
	void SetPosition(float x, float y, float z);
	void SetID(std::string ID);
	
	void AddPosition(float x, float y, float z);
	
	void Update();
	
	void Enable();
	void Disable();

	static void EnableLighting();
	static void DisableLighting();

	std::vector<float> GetParameters(GLenum type);
	
	GLenum GetLightNum();
	sf::Vector3f GetPosition();
	std::string GetID();
private:
	GLenum lightNum;
	sf::Vector3f position;
	std::string ID;
};

