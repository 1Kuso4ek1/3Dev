#include "Shape.h"

Shape::Shape(float w, float h, float d, float x, float y, float z) : size(w, h, d), position(x, y, z) {}

void Shape::Draw(GLuint texture) {
	glPushMatrix();
	glTranslatef(position.x, position.y, position.z);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(0.0, 0.0, 1.0); glVertex3f(-size.x, -size.y, -size.z);
	glTexCoord2f(1, 0); glVertex3f(size.x, -size.y, -size.z);
	glTexCoord2f(1, 1); glVertex3f(size.x, size.y, -size.z);
	glTexCoord2f(0, 1); glVertex3f(-size.x, size.y, -size.z);

	glTexCoord2f(0, 0); glNormal3f(-1.0, 0.0, 0.0); glVertex3f(size.x, -size.y, size.z);
	glTexCoord2f(1, 0); glVertex3f(-size.x, -size.y, size.z);
	glTexCoord2f(1, 1); glVertex3f(-size.x, size.y, size.z);
	glTexCoord2f(0, 1); glVertex3f(size.x, size.y, size.z);

	glTexCoord2f(0, 0); glNormal3f(0.0, 0.0, -1.0); glVertex3f(-size.x, -size.y, size.z);
	glTexCoord2f(1, 0); glVertex3f(-size.x, -size.y, -size.z);
	glTexCoord2f(1, 1); glVertex3f(-size.x, size.y, -size.z);
	glTexCoord2f(0, 1); glVertex3f(-size.x, size.y, size.z);

	glTexCoord2f(0, 0); glNormal3f(1.0, 0.0, 0.0); glVertex3f(size.x, -size.y, -size.z);
	glTexCoord2f(1, 0); glVertex3f(size.x, -size.y, size.z);
	glTexCoord2f(1, 1); glVertex3f(size.x, size.y, size.z);
	glTexCoord2f(0, 1); glVertex3f(size.x, size.y, -size.z);

	glTexCoord2f(0, 0); glNormal3f(0.0, 1.0, 0.0); glVertex3f(-size.x, -size.y, size.z);
	glTexCoord2f(1, 0); glVertex3f(size.x, -size.y, size.z);
	glTexCoord2f(1, 1); glVertex3f(size.x, -size.y, -size.z);
	glTexCoord2f(0, 1); glVertex3f(-size.x, -size.y, -size.z);

	glTexCoord2f(0, 0); glNormal3f(0.0, -1.0, 0.0); glVertex3f(-size.x, size.y, -size.z);
	glTexCoord2f(1, 0); glVertex3f(size.x, size.y, -size.z);
	glTexCoord2f(1, 1); glVertex3f(size.x, size.y, size.z);
	glTexCoord2f(0, 1); glVertex3f(-size.x, size.y, size.z);

	glEnd();
	glPopMatrix();
}

void Shape::Draw(GLuint texture[6])
{
	glPushMatrix();
	glTranslatef(position.x, position.y, position.z);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(0.0, 0.0, 1.0); glVertex3f(-size.x, -size.y, -size.z);
	glTexCoord2f(1, 0); glVertex3f(size.x, -size.y, -size.z);
	glTexCoord2f(1, 1); glVertex3f(size.x, size.y, -size.z);
	glTexCoord2f(0, 1); glVertex3f(-size.x, size.y, -size.z);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(-1.0, 0.0, 0.0); glVertex3f(size.x, -size.y, size.z);
	glTexCoord2f(1, 0); glVertex3f(-size.x, -size.y, size.z);
	glTexCoord2f(1, 1); glVertex3f(-size.x, size.y, size.z);
	glTexCoord2f(0, 1); glVertex3f(size.x, size.y, size.z);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(0.0, 0.0, -1.0); glVertex3f(-size.x, -size.y, size.z);
	glTexCoord2f(1, 0); glVertex3f(-size.x, -size.y, -size.z);
	glTexCoord2f(1, 1); glVertex3f(-size.x, size.y, -size.z);
	glTexCoord2f(0, 1); glVertex3f(-size.x, size.y, size.z);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(1.0, 0.0, 0.0); glVertex3f(size.x, -size.y, -size.z);
	glTexCoord2f(1, 0); glVertex3f(size.x, -size.y, size.z);
	glTexCoord2f(1, 1); glVertex3f(size.x, size.y, size.z);
	glTexCoord2f(0, 1); glVertex3f(size.x, size.y, -size.z);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(0.0, 1.0, 0.0); glVertex3f(-size.x, -size.y, size.z);
	glTexCoord2f(1, 0); glVertex3f(size.x, -size.y, size.z);
	glTexCoord2f(1, 1); glVertex3f(size.x, -size.y, -size.z);
	glTexCoord2f(0, 1); glVertex3f(-size.x, -size.y, -size.z);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(0.0, -1.0, 0.0); glVertex3f(-size.x, size.y, -size.z);
	glTexCoord2f(1, 0); glVertex3f(size.x, size.y, -size.z);
	glTexCoord2f(1, 1); glVertex3f(size.x, size.y, size.z);
	glTexCoord2f(0, 1); glVertex3f(-size.x, size.y, size.z);
	glEnd();
	glPopMatrix();
}

sf::Vector3f Shape::GetPosition() 
{
	return position;
}

sf::Vector3f Shape::GetSize() 
{
	return size;
}
