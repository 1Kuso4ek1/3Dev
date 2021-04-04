#pragma once
#include "3Dev.h"
class Shape {
public:
	Shape(float w, float h, float d, float x, float y, float z);

	static void Draw(GLuint texture, float x, float y, float z, float w, float h, float d);
	static void Draw(GLuint texture[6], float x, float y, float z, float w, float h, float d);

	void Draw(GLuint texture);
	void Draw(GLuint texture[6]);

	void DrawPlane(GLuint texture);
		
	void SetPosition(float x, float y, float z);
	void SetSize(float w, float h, float d);
	
	sf::Vector3f GetPosition();
	sf::Vector3f GetSize();
private:
	sf::Vector3f size;
	sf::Vector3f position;
};
