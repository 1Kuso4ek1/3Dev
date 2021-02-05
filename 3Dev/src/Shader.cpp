#include "Shader.h"

Shader::Shader(std::string filename, GLenum shadertype)
{
	std::ifstream shaderFile(filename);

	if(!shaderFile.is_open()) {
		std::cout << "ERROR::SHADER::CAN'T_OPEN_FILE" << std::endl;
		exit(-1);
	}

	std::getline(shaderFile, code, '\0');
	GLint success;
	GLuint shader = glCreateShader(shadertype);
	GLchar infoLog[512];
	const GLchar* temp = code.c_str();
	glShaderSource(shader, 1, &temp, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED" << std::endl << infoLog << std::endl;
	}

	program = glCreateProgram();
	glAttachShader(program, shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << std::endl << infoLog << std::endl;
	}

	glDeleteShader(shader);
}

void Shader::Bind() 
{
	glUseProgram(program);
}

void Shader::Unbind() 
{
	glUseProgram(NULL);
}

void Shader::SetUniform(std::string name, float val) 
{
	glUniform1f(glGetUniformLocation(program, name.c_str()), val);
}

void Shader::SetUniform(std::string name, float x, float y, float z)
{
	glUniform3f(glGetUniformLocation(program, name.c_str()), x, y, z);
}
