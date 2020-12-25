#include "Shape.h"

Shape::Shape(float w, float h, float d, float x, float y, float z) : w(w), h(h), d(d), x(x), y(y), z(z) {}

void Shape::Draw(GLuint texture) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(0.0, 0.0, 1.0); glVertex3f(-w, -h, -d);
	glTexCoord2f(1, 0); glVertex3f(w, -h, -d);
	glTexCoord2f(1, 1); glVertex3f(w, h, -d);
	glTexCoord2f(0, 1); glVertex3f(-w, h, -d);

	glTexCoord2f(0, 0); glNormal3f(-1.0, 0.0, 0.0); glVertex3f(d, -h, w);
	glTexCoord2f(1, 0); glVertex3f(-w, -h, d);
	glTexCoord2f(1, 1); glVertex3f(-w, h, d);
	glTexCoord2f(0, 1); glVertex3f(w, h, d);

	glTexCoord2f(0, 0); glNormal3f(0.0, 0.0, -1.0); glVertex3f(-w, -h, d);
	glTexCoord2f(1, 0); glVertex3f(-w, -h, -d);
	glTexCoord2f(1, 1); glVertex3f(-w, h, -d);
	glTexCoord2f(0, 1); glVertex3f(-w, h, d);

	glTexCoord2f(0, 0); glNormal3f(1.0, 0.0, 0.0); glVertex3f(w, -h, -d);
	glTexCoord2f(1, 0); glVertex3f(w, -h, d);
	glTexCoord2f(1, 1); glVertex3f(w, h, d);
	glTexCoord2f(0, 1); glVertex3f(w, h, -d);

	glTexCoord2f(0, 0); glNormal3f(0.0, 1.0, 0.0); glVertex3f(-w, -h, d);
	glTexCoord2f(1, 0); glVertex3f(w, -h, d);
	glTexCoord2f(1, 1); glVertex3f(w, -h, -d);
	glTexCoord2f(0, 1); glVertex3f(-w, -h, -d);

	glTexCoord2f(0, 0); glNormal3f(0.0, -1.0, 0.0); glVertex3f(-w, h, -d);
	glTexCoord2f(1, 0); glVertex3f(w, h, -d);
	glTexCoord2f(1, 1); glVertex3f(w, h, d);
	glTexCoord2f(0, 1); glVertex3f(-w, h, d);

	glEnd();
}

void Shape::Draw(GLuint texture[6])
{
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(0.0, 0.0, 1.0); glVertex3f(-w, -h, -d);
	glTexCoord2f(1, 0); glVertex3f(w, -h, -d);
	glTexCoord2f(1, 1); glVertex3f(w, h, -d);
	glTexCoord2f(0, 1); glVertex3f(-w, h, -d);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(-1.0, 0.0, 0.0); glVertex3f(d, -h, w);
	glTexCoord2f(1, 0); glVertex3f(-w, -h, d);
	glTexCoord2f(1, 1); glVertex3f(-w, h, d);
	glTexCoord2f(0, 1); glVertex3f(w, h, d);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(0.0, 0.0, -1.0); glVertex3f(-w, -h, d);
	glTexCoord2f(1, 0); glVertex3f(-w, -h, -d);
	glTexCoord2f(1, 1); glVertex3f(-w, h, -d);
	glTexCoord2f(0, 1); glVertex3f(-w, h, d);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(1.0, 0.0, 0.0); glVertex3f(w, -h, -d);
	glTexCoord2f(1, 0); glVertex3f(w, -h, d);
	glTexCoord2f(1, 1); glVertex3f(w, h, d);
	glTexCoord2f(0, 1); glVertex3f(w, h, -d);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(0.0, 1.0, 0.0); glVertex3f(-w, -h, d);
	glTexCoord2f(1, 0); glVertex3f(w, -h, d);
	glTexCoord2f(1, 1); glVertex3f(w, -h, -d);
	glTexCoord2f(0, 1); glVertex3f(-w, -h, -d);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glNormal3f(0.0, -1.0, 0.0); glVertex3f(-w, h, -d);
	glTexCoord2f(1, 0); glVertex3f(w, h, -d);
	glTexCoord2f(1, 1); glVertex3f(w, h, d);
	glTexCoord2f(0, 1); glVertex3f(-w, h, d);
	glEnd();
}

void Shape::Draw(sf::Color color)
{
	glBegin(GL_QUADS);
	glColor3f(color.r, color.g, color.b);
	glNormal3f(0.0, 0.0, 1.0); glVertex3f(-w, -h, -d);
	glVertex3f(w, -h, -d);
	glVertex3f(w, h, -d);
	glVertex3f(-w, h, -d);

	glNormal3f(-1.0, 0.0, 0.0); glVertex3f(d, -h, w);
	glVertex3f(-w, -h, d);
	glVertex3f(-w, h, d);
	glVertex3f(w, h, d);

	glNormal3f(0.0, 0.0, -1.0); glVertex3f(-w, -h, d);
	glVertex3f(-w, -h, -d);
	glVertex3f(-w, h, -d);
	glVertex3f(-w, h, d);

	glNormal3f(1.0, 0.0, 0.0); glVertex3f(w, -h, -d);
	glVertex3f(w, -h, d);
	glVertex3f(w, h, d);
	glVertex3f(w, h, -d);

	glNormal3f(0.0, 1.0, 0.0); glVertex3f(-w, -h, d);
	glVertex3f(w, -h, d);
	glVertex3f(w, -h, -d);
	glVertex3f(-w, -h, -d);

	glNormal3f(0.0, -1.0, 0.0); glVertex3f(-w, h, -d);
	glVertex3f(w, h, -d);
	glVertex3f(w, h, d);
	glVertex3f(-w, h, d);

	glEnd();
}
