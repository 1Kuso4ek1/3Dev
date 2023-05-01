#pragma once
#include "Utils.hpp"

class Shader {
public:
	Shader(const std::string& vert, const std::string& frag, bool load = true);

	void Bind();
	static void Unbind();

	void SetUniform1i(const std::string& name, int val);
	void SetUniform1f(const std::string& name, float val);
	void SetUniform2f(const std::string& name, float x, float y);
	void SetUniform3f(const std::string& name, float x, float y, float z);

	void SetUniformMatrix4(const std::string& name, glm::mat4 mat);
	void SetVectorOfUniformMatrix4(const std::string& name, int count, std::vector<glm::mat4>& mat);

	int GetUniformLocation(const std::string& name);
	int GetAttribLocation(const std::string& name);

private:
	void Compile();

	GLuint program;

	std::string vCode, fCode;

	std::unordered_map<std::string, GLuint> cache;
};
