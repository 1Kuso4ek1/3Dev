#pragma once
#include "Texture.h"
class Model
{
private:
	float* vertexArray;
	float* normalArray;
	float* uvArray;
	int numVerts;
	GLuint ModelTexture;
public:
	bool Load(std::string filename, std::string texture); //Load with texture
	bool Load(std::string filename); //Load without texture
	void Draw(bool tex = true); //Draw with texture, or if bool tex = false, draw without texture
	void Draw(GLuint texture); //Draw with another texture
};

