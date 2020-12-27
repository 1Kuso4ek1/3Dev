#pragma once
#include "3Dev.h"
class Shape {
public:
	Shape(float w, float h, float d, float x, float y, float z);

	void Draw(GLuint texture);
	void Draw(GLuint texture[6]);
private:
	float w, h, d;
	float x, y, z;
};
