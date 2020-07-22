#pragma once
#include "3Dev.h"
class Shader {
private:
	GLuint Program;

	std::string vertexCode;
	std::string fragmentCode;

	std::ifstream vShaderFile; //Vertex shader input file
	std::ifstream fShaderFile; //Fragment shader input file
public:
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath); //Loading shader
	void Use(); //Using shader
	void SetUniform(std::string name, float val); 
	void SetUniform(std::string name, float x, float y, float z);
};