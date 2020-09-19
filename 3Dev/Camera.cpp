#include "Camera.h"

Camera::Camera(float x, float y, float z, float speed) : x(x), y(y), z(z), speed(speed) {}

void Camera::Move(float time)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		x += -sin(angleX / 180 * pi) * speed * time;
		y += tan(angleY / 180 * pi) * speed * time;
		z += -cos(angleX / 180 * pi) * speed * time;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		x += sin(angleX / 180 * pi) * speed * time;
		y += -tan(angleY / 180 * pi) * speed * time;
		z += cos(angleX / 180 * pi) * speed * time;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		x += sin((angleX + 90) / 180 * pi) * speed * time;
		z += cos((angleX + 90) / 180 * pi) * speed * time;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		x += sin((angleX - 90) / 180 * pi) * speed * time;
		z += cos((angleX - 90) / 180 * pi) * speed * time;
	}
}

void Camera::Mouse(float WindowPosX, float WindowPosY, float WindowX, float WindowY)
{
	POINT mousexy;
	GetCursorPos(&mousexy);
	int xt = WindowPosX + WindowX / 2;
	int yt = WindowPosY + WindowY / 2;

	angleX += (xt - mousexy.x) / 4;
	angleY += (yt - mousexy.y) / 4;
	if (angleY < -89.0) {
		angleY = -89.0;
	}
	if (angleY > 89.0) {
		angleY = 89.0;
	}
	SetCursorPos(xt, yt);
}

void Camera::Look()
{
	gluLookAt(x, y, z, x - sin(angleX / 180 * pi), y + tan(angleY / 180 * pi), z - cos(angleX / 180 * pi), 0, 1, 0);
}

void Camera::Look(float x, float y, float z)
{
	gluLookAt(x, y, z, x - sin(angleX / 180 * pi), y + tan(angleY / 180 * pi), z - cos(angleX / 180 * pi), 0, 1, 0);
}
