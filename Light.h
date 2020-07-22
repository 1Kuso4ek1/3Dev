#pragma once
#include "3Dev.h"
class Light
{
private:
	GLenum lightNum;
public:
	Light(GLenum lightNum);
	void SetParameters(float parameters[4], GLenum type);
};

