#pragma once
#include "Model.h"

class Animation
{
public:
	Animation(int frames, std::string filename, std::string texture, float x, float y, float z, float speed, std::string ID, float rx, float ry, float rz, float sx, float sy, float sz); //Constructor
	~Animation(); //Destructor

	float sizeX = 1, sizeY = 1, sizeZ = 1;
	float rotationX = 0, rotationY = 0, rotationZ = 0;
	float speed;
	float x, y, z;

	int frames; // frames

	std::string ID, filename, stexture;

	void DrawAnimation(float time); //Draw 1 frame with texture
	void SetPosition(float x, float y, float z);
	void SetSize(float sizeX, float sizeY, float sizeZ);
	void SetRotation(float rotationX, float rotationY, float rotationZ);
	void AddPosition(float x, float y, float z);
	void AddSize(float sizeX, float sizeY, float sizeZ);
	void AddRotation(float rotationX, float rotationY, float rotationZ);
private:
	Model* m; //Model for each frame
	float current_frame = 1;
	GLuint texture;
};

