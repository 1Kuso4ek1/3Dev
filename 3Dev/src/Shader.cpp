#include "Shader.h"

Shader::Shader(std::string vertname, std::string fragname)
{
	std::ifstream vertfile(vertname);
	std::ifstream fragfile(fragname);

	if(!vertfile.is_open())
		Log::Write("Can't open file '" + vertname + "'!", Log::Type::Critical);
	if(!fragfile.is_open())
		Log::Write("Can't open file '" + fragname + "'!", Log::Type::Critical);

	std::getline(vertfile, vertcode, '\0');
	std::getline(fragfile, fragcode, '\0');

	GLint success;
	GLuint vertshader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragshader = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar log[512];

	const GLchar* verttemp = vertcode.c_str();
	const GLchar* fragtemp = fragcode.c_str();
	
	glShaderSource(vertshader, 1, &verttemp, NULL);
	glShaderSource(fragshader, 1, &fragtemp, NULL);
	
	glCompileShader(vertshader);
	glCompileShader(fragshader);

	glGetShaderiv(vertshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertshader, 512, NULL, log);
		Log::Write("Vertex shader compilation failed! Reason: " + std::string(log), Log::Type::Critical);
	}

	glGetShaderiv(fragshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragshader, 512, NULL, log);
		Log::Write("Fragment shader compilation failed! Reason: " + std::string(log), Log::Type::Critical);
	}

	program = glCreateProgram();
	
	glAttachShader(program, vertshader);
	glAttachShader(program, fragshader);
	
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, log);
		Log::Write("Program linking failed! Reason: " + std::string(log), Log::Type::Critical);
	}

	glDeleteShader(vertshader);
	glDeleteShader(fragshader);
}

void Shader::Bind() 
{
	glUseProgram(program);
}

void Shader::Unbind() 
{
	glUseProgram(0);
}

void Shader::SetUniform1i(std::string name, int val)
{
	glUniform1i(GetUniformLocation(name), val);
}

void Shader::SetUniform1f(std::string name, float val) 
{
	glUniform1f(GetUniformLocation(name), val);
}

void Shader::SetUniform3f(std::string name, float x, float y, float z)
{
	glUniform3f(GetUniformLocation(name), x, y, z);
}

void Shader::SetUniform2f(std::string name, float x, float y)
{
	glUniform2f(GetUniformLocation(name), x, y);
}

void Shader::SetUniformMatrix4(std::string name, glm::mat4 mat)
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, 0, glm::value_ptr(mat));
}

int Shader::GetUniformLocation(std::string name)
{
	return glGetUniformLocation(program, name.c_str());
}

int Shader::GetAttribLocation(std::string name)
{
	return glGetAttribLocation(program, name.c_str());
}
