#pragma once
#include "3Dev.h"
class Shape {
public:
	Shape(float w, float h, float d, float x, float y, float z);

	void Draw(GLuint texture);
	void Draw(GLuint texture[6]);
	
	sf::Vector3f GetPosition();
	sf::Vector3f GetSize();
private:
	sf::Vector3f size;
	sf::Vector3f position;
};
