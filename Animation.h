#pragma once
#include "Model.h"

class Animation
{
private:
	Model* m; //Model for each frame
	int frames; // frames
public:
	Animation(int frames); //Constructor
	~Animation(); //Destructor
	void LoadAnimation(std::string filename); //Function for loading animation
	void DrawAnimation(int frame, GLuint texture); //Draw 1 frame with texture
	void DrawAnimation(int frame); //Draw 1 frame without texture
};

