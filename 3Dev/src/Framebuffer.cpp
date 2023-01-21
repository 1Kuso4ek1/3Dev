#include <Framebuffer.hpp>

Framebuffer::Framebuffer(Shader* shader, int w, int h, bool isDepth, GLint filter, GLint wrap) : shader(shader), size(w, h)
{
	CalcPixelSize(glm::vec2(w, h));
    if(!isDepth) texture = TextureManager::GetInstance()->CreateTexture(w, h, false, filter, wrap);
	
	depth = TextureManager::GetInstance()->CreateTexture(w, h, true, isDepth ? GL_LINEAR : GL_NEAREST, wrap);
    glGenFramebuffers(1, &fbo);
    
	Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    
	if(!isDepth) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	else glDrawBuffer(GL_NONE);
	
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        Log::Write("framebuffer status isn't GL_FRAMEBUFFER_COMPLETE", Log::Type::Error);
	Unbind();

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (void*)12);

	glBindVertexArray(0);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &fbo);
}

void Framebuffer::RecreateTexture(int w, int h)
{
	size = glm::ivec2(w, h);
	CalcPixelSize(glm::vec2(w, h));

	if(texture != 0)
	{
		auto name = TextureManager::GetInstance()->GetName(texture);
		TextureManager::GetInstance()->DeleteTexture(name);
		texture = TextureManager::GetInstance()->CreateTexture(w, h);
	}
	auto name = TextureManager::GetInstance()->GetName(depth);
	TextureManager::GetInstance()->DeleteTexture(name);
	depth = TextureManager::GetInstance()->CreateTexture(w, h, true);

    Bind();
    if(texture != 0) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        Log::Write("framebuffer status isn't GL_FRAMEBUFFER_COMPLETE", Log::Type::Error);
    Unbind();
}

void Framebuffer::CalcPixelSize(glm::vec2 v)
{
	pixelsize = glm::vec2(1.0) / v;
}

void Framebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Draw()
{
	shader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	shader->SetUniform1i("frame", 0);
	shader->SetUniform2f("pixelsize", pixelsize.x, pixelsize.y);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	shader->Unbind();
}

GLuint Framebuffer::Capture(GLuint texture)
{
	GLuint out = TextureManager::GetInstance()->CreateTexture(size.x, size.y);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out, 0);
	Bind();
	glViewport(0, 0, size.x, size.y);
	shader->Bind();
	if(texture != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		shader->SetUniform1i("frame", 0);
		shader->SetUniform2f("pixelsize", pixelsize.x, pixelsize.y);
	}
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	shader->Unbind();
	Unbind();
	return out;
}

GLuint Framebuffer::CaptureCubemap(Shader* shader, GLuint tex, Matrices& m, bool isSkybox)
{
	Mesh cube;
	cube.CreateCube();
	GLuint cubemap = TextureManager::GetInstance()->CreateCubemap(size.x, GL_LINEAR_MIPMAP_LINEAR);
	m.PushMatrix();
	m.GetProjection() = glm::perspective(glm::radians(90.0), 1.0, 0.1, 1000.0);
	Bind();
	glViewport(0, 0, size.x, size.x);
	for(int i = 0; i < 6; i++)
	{
		m.GetView() = views[i];
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, 0);
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if(isSkybox)
		{
			glDepthFunc(GL_LEQUAL);
			glDisable(GL_CULL_FACE);

			m.PushMatrix();

			shader->Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

			shader->SetUniform1i("cubemap", 0);
			m.UpdateShader(shader);

			cube.Draw();

			m.PopMatrix();

			glEnable(GL_CULL_FACE);
			glDepthFunc(GL_LESS);
		}
		else
		{
			m.PushMatrix();

			shader->Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex);

			shader->SetUniform1i("environment", 0);
			m.UpdateShader(shader);

			cube.Draw();

			m.PopMatrix();
		}
	}
	Unbind();
	m.PopMatrix();
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return cubemap;
}

GLuint Framebuffer::CaptureCubemapMipmaps(Shader* shader, GLuint tex, Matrices& m, int maxLevel, int samples)
{
	Mesh cube;
	cube.CreateCube();
	GLuint cubemap = TextureManager::GetInstance()->CreateCubemap(size.x, GL_LINEAR_MIPMAP_LINEAR);
	m.PushMatrix();
	m.GetProjection() = glm::perspective(glm::radians(90.0), 1.0, 0.1, 1000.0);
	Bind();
	shader->Bind();
	shader->SetUniform1i("samples", samples);
	for(int i = 0; i < maxLevel; i++)
	{
		shader->SetUniform1f("roughness", (float)i / (float)(maxLevel - 1));
		glm::ivec2 msize = size / (int)std::pow(2, i);
		glViewport(0, 0, msize.x, msize.x);
		for(int j = 0; j < 6; j++)
		{
			m.GetView() = views[j];
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, cubemap, i);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDepthFunc(GL_LEQUAL);
			glDisable(GL_CULL_FACE);

			m.PushMatrix();

			shader->Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

			shader->SetUniform1i("cubemap", 0);
			m.UpdateShader(shader);

			cube.Draw();

			m.PopMatrix();

			glEnable(GL_CULL_FACE);
			glDepthFunc(GL_LESS);
		}
	}
	Unbind();
	m.PopMatrix();
	return cubemap;
}

float* Framebuffer::GetPixels(glm::ivec2 coords, glm::ivec2 size)
{
    float* data = (float*)malloc(size.x * size.y * 4 * 4);
    glReadPixels(coords.x, coords.y, size.x, size.y, GL_RGBA, GL_FLOAT, data);
    return data;
}

GLuint Framebuffer::GetTexture(bool depth)
{
	return (depth ? this->depth : texture);
}

glm::ivec2 Framebuffer::GetSize()
{
	return size;
}
