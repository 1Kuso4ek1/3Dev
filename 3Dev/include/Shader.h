#pragma once
#include "3Dev.h"

class Shader {
public:
	Shader(std::string vertname, std::string fragname);

	void Bind();
	void Unbind();

	void SetUniform1i(std::string name, int val); 
	void SetUniform1f(std::string name, float val);
	void SetUniform2f(std::string name, float x, float y);
	void SetUniform3f(std::string name, float x, float y, float z);

	void SetUniformMatrix4(std::string name, glm::mat4 mat);

	int GetUniformLocation(std::string name);
	int GetAttribLocation(std::string name);

private:
	GLuint program;

	std::string vertcode, fragcode;
};
