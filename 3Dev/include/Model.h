#pragma once
#include "Texture.h"
class Model
{
public:
	float x, y, z;
	float rotationX = 0, rotationY = 0, rotationZ = 0;
	float sizeX = 1, sizeY = 1, sizeZ = 1;

	float* vertexArray;
	
	std::string ID, filename, stexture;
	
	Model(std::string filename, std::string texture, std::string ID, float x, float y, float z, float rotationX, float rotationY, float rotationZ, float sizeX, float sizeY, float sizeZ);
	Model(std::string filename, std::string texture, std::string ID, float x, float y, float z);
	Model(std::string filename, std::string ID, float x, float y, float z);
	
	Model();
	~Model();

	bool Load(std::string filename, std::string texture); //Load with texture
	bool Load(std::string filename); //Load without texture

	void Draw(bool tex = true); //Draw with texture, or if bool tex = false, draw without texture
	void Draw(GLuint texture); //Draw with another texture
	void SetPosition(float x, float y, float z);
	void SetSize(float sizeX, float sizeY, float sizeZ);
	void SetRotation(float rotationX, float rotationY, float rotationZ);

private:
	float* normalArray;
	float* uvArray;

	int numVerts;
	GLuint ModelTexture;
};

