#pragma once
#include "3Dev.h"

class Shader {
public:
	Shader(std::string filename, GLenum shadertype);

	void Bind();
	void Unbind();
	void SetUniform(std::string name, float val); 
	void SetUniform(std::string name, float x, float y, float z);
private:
	GLuint program;

	std::string code;
};
