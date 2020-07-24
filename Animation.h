#pragma once
#include "Model.h"

class Animation
{
private:
	Model* m; //Model for each frame
	float current_frame = 1;
	int frames; // frames
public:
	Animation(int frames); //Constructor
	~Animation(); //Destructor
	void LoadAnimation(std::string filename); //Function for loading animation
	void DrawAnimation(float speed, GLuint texture); //Draw 1 frame with texture
	void DrawAnimation(float speed); //Draw 1 frame without texture
};

