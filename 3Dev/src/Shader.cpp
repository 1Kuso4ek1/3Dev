#include "Shader.hpp"

Shader::Shader(const std::string& vert, const std::string& frag, bool load)
{
	if(load)
	{
		std::ifstream vFile(vert);
		std::ifstream fFile(frag);

		if(!vFile.is_open())
			Log::Write("Can't open file '" + vert + "'!", Log::Type::Critical);
		if(!fFile.is_open())
			Log::Write("Can't open file '" + frag + "'!", Log::Type::Critical);

		std::getline(vFile, vCode, '\0');
		std::getline(fFile, fCode, '\0');
	}
	else
	{
		vCode = vert; fCode = frag;
	}

	Compile();
}

void Shader::Bind()
{
	glUseProgram(program);
}

void Shader::Unbind()
{
	glUseProgram(0);
}

void Shader::SetUniform1i(const std::string& name, int val)
{
	glUniform1i(GetUniformLocation(name), val);
}

void Shader::SetUniform1f(const std::string& name, float val)
{
	glUniform1f(GetUniformLocation(name), val);
}

void Shader::SetUniform3f(const std::string& name, float x, float y, float z)
{
	glUniform3f(GetUniformLocation(name), x, y, z);
}

void Shader::SetUniform2f(const std::string& name, float x, float y)
{
	glUniform2f(GetUniformLocation(name), x, y);
}

void Shader::SetUniformMatrix4(const std::string& name, glm::mat4 mat)
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, 0, glm::value_ptr(mat));
}

void Shader::SetVectorOfUniformMatrix4(const std::string& name, int count, std::vector<glm::mat4>& mat)
{
	glUniformMatrix4fv(GetUniformLocation(name), count, 0, glm::value_ptr(mat[0]));
}

int Shader::GetUniformLocation(const std::string& name)
{
    if(cache.find(name) != cache.end())
        return cache[name];
    return (cache[name] = glGetUniformLocation(program, name.c_str()));
}

int Shader::GetAttribLocation(const std::string& name)
{
	return glGetAttribLocation(program, name.c_str());
}

void Shader::Compile()
{
	GLint success;
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar log[512];

	const GLchar* vTemp = vCode.c_str();
	const GLchar* fTemp = fCode.c_str();

	glShaderSource(vShader, 1, &vTemp, NULL);
	glShaderSource(fShader, 1, &fTemp, NULL);

	glCompileShader(vShader);
	glCompileShader(fShader);

	glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vShader, 512, NULL, log);
		Log::Write("Vertex shader compilation failed! Reason: " + std::string(log), Log::Type::Critical);
	}

	glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fShader, 512, NULL, log);
		Log::Write("Fragment shader compilation failed! Reason: " + std::string(log), Log::Type::Critical);
	}

	program = glCreateProgram();

	glAttachShader(program, vShader);
	glAttachShader(program, fShader);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, log);
		Log::Write("Program linking failed! Reason: " + std::string(log), Log::Type::Critical);
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);
}
