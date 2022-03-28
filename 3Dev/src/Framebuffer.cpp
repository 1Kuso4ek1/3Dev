#include <Framebuffer.h>

Framebuffer::Framebuffer(Shader* shader, int w, int h) : shader(shader)
{
	CalcPixelSize(glm::vec2(w, h));
    texture = CreateTexture(w, h);
	depth = CreateTexture(w, h, true);
    glGenFramebuffers(1, &fbo);
    Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Error: framebuffer status isn't GL_FRAMEBUFFER_COMPLETE" << std::endl;
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
	CalcPixelSize(glm::vec2(w, h));
    texture = CreateTexture(w, h);
	depth = CreateTexture(w, h, true);
    Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Error: framebuffer status isn't GL_FRAMEBUFFER_COMPLETE" << std::endl;
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
    glBindFramebuffer(GL_FRAMEBUFFER, GLuint(0));
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

GLuint Framebuffer::GetTexture(bool depth)
{
	return (depth ? this->depth : texture);
}