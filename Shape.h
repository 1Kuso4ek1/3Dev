#pragma once
#include "3Dev.h"
void RenderShape(GLuint texture, float x, float y, float z) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(0.0, 0.0, 1.0); glVertex3f(-x, -y, -z);
	glTexCoord2f(1, 0); glVertex3f(x, -y, -z);
	glTexCoord2f(1, 1); glVertex3f(x, y, -z);
	glTexCoord2f(0, 1); glVertex3f(-x, y, -z);

	glTexCoord2f(0, 0); glNormal3f(-1.0, 0.0, 0.0); glVertex3f(x, -y, z);
	glTexCoord2f(1, 0); glVertex3f(-x, -y, z);
	glTexCoord2f(1, 1); glVertex3f(-x, y, z);
	glTexCoord2f(0, 1); glVertex3f(x, y, z);

	glTexCoord2f(0, 0); glNormal3f(0.0, 0.0, -1.0); glVertex3f(-x, -y, z);
	glTexCoord2f(1, 0); glVertex3f(-x, -y, -z);
	glTexCoord2f(1, 1); glVertex3f(-x, y, -z);
	glTexCoord2f(0, 1); glVertex3f(-x, y, z);

	glTexCoord2f(0, 0); glNormal3f(1.0, 0.0, 0.0); glVertex3f(x, -y, -z);
	glTexCoord2f(1, 0); glVertex3f(x, -y, z);
	glTexCoord2f(1, 1); glVertex3f(x, y, z);
	glTexCoord2f(0, 1); glVertex3f(x, y, -z);

	glTexCoord2f(0, 0); glNormal3f(0.0, 1.0, 0.0); glVertex3f(-x, -y, z);
	glTexCoord2f(1, 0); glVertex3f(x, -y, z);
	glTexCoord2f(1, 1); glVertex3f(x, -y, -z);
	glTexCoord2f(0, 1); glVertex3f(-x, -y, -z);

	glTexCoord2f(0, 0); glNormal3f(0.0, -1.0, 0.0); glVertex3f(-x, y, -z);
	glTexCoord2f(1, 0); glVertex3f(x, y, -z);
	glTexCoord2f(1, 1); glVertex3f(x, y, z);
	glTexCoord2f(0, 1); glVertex3f(-x, y, z);

	glEnd();
}
