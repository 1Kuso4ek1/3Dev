#pragma once
#include "3Dev.h"

class Light
{
public:
	Light(GLenum lightNum, float x, float y, float z, std::string ID);
	Light(GLenum lightNum, float x, float y, float z);

	~Light();
	
	void SetParameters(float parameters[], GLenum type);
	void SetPosition(float x, float y, float z);
	void SetID(std::string ID);
	
	void Update();
	
	void GetParameters(GLenum type, float* var);
	GLenum GetLightNum();
	sf::Vector3f GetPosition();
	std::string GetID();
private:
	GLenum lightNum;
	sf::Vector3f position;
	std::string ID;
};

