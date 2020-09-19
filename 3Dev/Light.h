#pragma once
#include "3Dev.h"
class Light
{
public:
	float x, y, z;

	Light(GLenum lightNum, float x, float y, float z);

	void SetParameters(float parameters[], GLenum type);
	void SetPosition(float x, float y, float z);
private:
	GLenum lightNum;
};

