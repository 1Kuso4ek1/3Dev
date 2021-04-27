#pragma once
#include "3Dev.h"
class Shape {
public:
	Shape(float w, float h, float d, float x, float y, float z);

	static void Draw(GLuint texture, float x, float y, float z, float w, float h, float d, sf::Vector3f rotation = sf::Vector3f(0, 0, 0));
	static void Draw(GLuint texture[6], float x, float y, float z, float w, float h, float d, sf::Vector3f rotation = sf::Vector3f(0, 0, 0));

	void Draw(GLuint texture);
	void Draw(GLuint texture[6]);

	void DrawPlane(GLuint texture);
		
	void SetPosition(float x, float y, float z);
	void SetRotation(float rotationX, float rotationY, float rotationZ);
	void SetSize(float w, float h, float d);
	
	sf::Vector3f GetPosition();
	sf::Vector3f GetRotation();
	sf::Vector3f GetSize();
private:
	sf::Vector3f size;
	sf::Vector3f position;
	sf::Vector3f rotation;
};
