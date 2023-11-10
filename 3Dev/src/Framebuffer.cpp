#include <Framebuffer.hpp>

Framebuffer::Framebuffer(Shader* shader, int w, int h, bool isDepth, bool createDepthAttachment, uint32_t attachments, GLint filter, GLint wrap) : shader(shader), size(w, h)
{
	CalcPixelSize(glm::vec2(w, h));
    if(!isDepth)
		for(int i = 0; i < attachments; i++)
			textures.push_back(TextureManager::GetInstance()->CreateTexture(w, h, false, filter, wrap));
		
    glGenFramebuffers(1, &fbo);
    
	Bind();

	if(createDepthAttachment)
	{
		depth = TextureManager::GetInstance()->CreateTexture(w, h, true, filter, wrap);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
	}
    
	if(!isDepth)
	{
		std::vector<GLenum> tmp;
		for(int i = 0; i < attachments; i++)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
			tmp.push_back(GL_COLOR_ATTACHMENT0 + i);
		}

		glDrawBuffers(attachments, &tmp[0]);
	}
	else glDrawBuffer(GL_NONE);
	
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        Log::Write("Failed to create framebuffer: " + errorStatus[status], Log::Type::Error);
	Unbind();

	rectangle = std::make_shared<Mesh>();
	rectangle->CreateRectangle();
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &fbo);
}

void Framebuffer::Resize(int w, int h)
{
	size = glm::ivec2(w, h);
	CalcPixelSize(glm::vec2(w, h));

	if(!textures.empty())
	{
		for(auto& i : textures)
		{
			auto name = TextureManager::GetInstance()->GetName(i);
			TextureManager::GetInstance()->ResizeTexture(name, false, w, h);
		}
	}
	
	if(depth)
	{
		auto name = TextureManager::GetInstance()->GetName(depth);
		TextureManager::GetInstance()->ResizeTexture(name, true, w, h);
	}
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

void Framebuffer::Draw(uint32_t attachment)
{
	shader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[attachment]);
	shader->SetUniform1i("frame", 0);
	shader->SetUniform2f("pixelsize", pixelsize.x, pixelsize.y);
	rectangle->Draw();
	shader->Unbind();
}

GLuint Framebuffer::Capture(GLuint texture, GLuint output)
{
	if(output == 0)
		output = TextureManager::GetInstance()->CreateTexture(size.x, size.y);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output, 0);
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
	rectangle->Draw();
	shader->Unbind();
	Unbind();
	return output;
}

GLuint Framebuffer::CaptureCubemap(Shader* shader, GLuint tex, Matrices& m, bool isSkybox, GLuint output)
{
	Mesh cube;
	cube.CreateCube();
	if(output == 0)
		output = TextureManager::GetInstance()->CreateCubemap(size.x, GL_LINEAR_MIPMAP_LINEAR);
	m.PushMatrix();
	m.GetProjection() = glm::perspective(glm::radians(90.0), 1.0, 0.1, 1000.0);
	Bind();
	glViewport(0, 0, size.x, size.x);
	for(int i = 0; i < 6; i++)
	{
		m.GetView() = views[i];
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, output, 0);
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
	glBindTexture(GL_TEXTURE_CUBE_MAP, output);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return output;
}

GLuint Framebuffer::CaptureCubemapMipmaps(Shader* shader, GLuint tex, Matrices& m, int maxLevel, int samples, GLuint output)
{
	Mesh cube;
	cube.CreateCube();
	if(output == 0)
		output = TextureManager::GetInstance()->CreateCubemap(size.x, GL_LINEAR_MIPMAP_LINEAR);
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
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, output, i);
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
	return output;
}

float* Framebuffer::GetPixels(glm::ivec2 coords, glm::ivec2 size)
{
    float* data = (float*)malloc(size.x * size.y * 4 * 4);
    glReadPixels(coords.x, coords.y, size.x, size.y, GL_RGBA, GL_FLOAT, data);
    return data;
}

GLuint Framebuffer::GetTexture(bool depth, uint32_t attachment)
{
	return (depth ? this->depth : textures[attachment]);
}

glm::ivec2 Framebuffer::GetSize()
{
	return size;
}
