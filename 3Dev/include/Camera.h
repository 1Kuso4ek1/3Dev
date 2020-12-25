#pragma once
#include "3Dev.h"
class Camera {
public:
	float x, y, z, speed;
	float angleX = 0, angleY = 0;

	sf::Vector2f oldmousexy;

	Camera(float x, float y, float z, float speed);

	void Move(float time);
	void Mouse(sf::RenderWindow& window);
	void Look();
	void Look(float x, float y, float z);
};
