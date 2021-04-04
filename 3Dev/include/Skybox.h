#pragma once
#include "3Dev.h"

[[deprecated("Use static void Shape::Draw(GLuint, float, float, float, float, float, float) or static void Shape::Draw(GLuint*, float, float, float, float, float, float) instead")]]
void RenderSkybox(GLuint skybox[], float size)
{
	glBindTexture(GL_TEXTURE_2D, skybox[0]);
	glBegin(GL_QUADS);
	//front
	glTexCoord2f(0, 0);   glVertex3f(-size, -size, -size);
	glTexCoord2f(1, 0);   glVertex3f(size, -size, -size);
	glTexCoord2f(1, 1);   glVertex3f(size, size, -size);
	glTexCoord2f(0, 1);   glVertex3f(-size, size, -size);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, skybox[1]);
	glBegin(GL_QUADS);
	//back
	glTexCoord2f(0, 0); glVertex3f(size, -size, size);
	glTexCoord2f(1, 0); glVertex3f(-size, -size, size);
	glTexCoord2f(1, 1); glVertex3f(-size, size, size);
	glTexCoord2f(0, 1); glVertex3f(size, size, size);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, skybox[2]);
	glBegin(GL_QUADS);
	//left
	glTexCoord2f(0, 0); glVertex3f(-size, -size, size);
	glTexCoord2f(1, 0); glVertex3f(-size, -size, -size);
	glTexCoord2f(1, 1); glVertex3f(-size, size, -size);
	glTexCoord2f(0, 1); glVertex3f(-size, size, size);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, skybox[3]);
	glBegin(GL_QUADS);
	//right
	glTexCoord2f(0, 0); glVertex3f(size, -size, -size);
	glTexCoord2f(1, 0); glVertex3f(size, -size, size);
	glTexCoord2f(1, 1); glVertex3f(size, size, size);
	glTexCoord2f(0, 1); glVertex3f(size, size, -size);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, skybox[4]);
	glBegin(GL_QUADS);
	//bottom
	glTexCoord2f(0, 0); glVertex3f(-size, -size, size);
	glTexCoord2f(1, 0); glVertex3f(size, -size, size);
	glTexCoord2f(1, 1); glVertex3f(size, -size, -size);
	glTexCoord2f(0, 1); glVertex3f(-size, -size, -size);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, skybox[5]);
	glBegin(GL_QUADS);
	//top  		
	glTexCoord2f(0, 0); glVertex3f(-size, size, -size);
	glTexCoord2f(1, 0); glVertex3f(size, size, -size);
	glTexCoord2f(1, 1); glVertex3f(size, size, size);
	glTexCoord2f(0, 1); glVertex3f(-size, size, size);
	glEnd();
}
