#pragma once
#include <Model.h>

class Animation
{
public:
	Animation(int frames, std::string filename, std::string texture, float x, float y, float z, float speed, std::string ID, float rx, float ry, float rz, float sx, float sy, float sz); //Constructor
	~Animation();

	void DrawAnimation(float time);
	void DrawFrame(int frame);
	
	void SetPosition(float x, float y, float z);
	void SetSize(float sizeX, float sizeY, float sizeZ);
	void SetRotation(float rotationX, float rotationY, float rotationZ);
	void SetID(std::string ID);
	
	void AddPosition(float x, float y, float z);
	void AddSize(float sizeX, float sizeY, float sizeZ);
	void AddRotation(float rotationX, float rotationY, float rotationZ);
	
	sf::Vector3f GetPosition();
	sf::Vector3f GetRotation();
	sf::Vector3f GetSize();
	
	std::string GetID();
	std::string GetFilename();
	std::string GetTextureFilename();
	
	int GetFrames();
private:
	sf::Vector3f position, rotation, size;

	int frames;

	std::string ID, filename, texture;

	Model* m;
	
	float current_frame = 1;
	float speed;
	
	GLuint animationTexture;
};

