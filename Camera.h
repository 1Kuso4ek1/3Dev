#pragma once
#include "3Dev.h"
class Camera {
public:
	float x, y, z, speed;
	float angleX = 0, angleY = 0;
	Camera(float x, float y, float z, float speed);
	void Move(float time);
	void Mouse(float WindowPosX, float WindowPosY, float WindowX, float WindowY);
	void Look();
	void Look(float x, float y, float z);
};