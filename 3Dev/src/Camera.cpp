#include "Camera.h"

Camera::Camera(float x, float y, float z, float speed) : x(x), y(y), z(z), speed(speed) {}

void Camera::Move(float time)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		x += -sin(angleX / 180 * pi) * (speed * time);
		y += tan(angleY / 180 * pi) * (speed * time);
		z += -cos(angleX / 180 * pi) * (speed * time);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		x += sin(angleX / 180 * pi) * (speed * time);
		y += -tan(angleY / 180 * pi) * (speed * time);
		z += cos(angleX / 180 * pi) * (speed * time);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		x += sin((angleX + 90) / 180 * pi) * (speed * time);
		z += cos((angleX + 90) / 180 * pi) * (speed * time);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		x += sin((angleX - 90) / 180 * pi) * (speed * time);
		z += cos((angleX - 90) / 180 * pi) * (speed * time);
	}
}

void Camera::ThirdPerson(float centerx, float centery, float centerz, float dist)
{
	float horizontald = dist * cos(angleY * pi / 180);
	float verticald = dist * sin(angleY * pi / 180);
	x = centerx - (horizontald * sin(angleX * pi / 180));
	y = verticald + centery;
	z = centerz - (horizontald * cos(angleX * pi / 180));
}

void Camera::Mouse(sf::RenderWindow& window)
{
	sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
	sf::Vector2f mousexy = window.mapPixelToCoords(pixelPos, sf::View({ (float)window.getSize().x / 2, (float)window.getSize().y / 2 }, 
                                        									{ (float)window.getSize().x, (float)window.getSize().y }));
	
	angleX -= (mousexy.x - window.getSize().x / 2) / 8;
	angleY -= (mousexy.y - window.getSize().y / 2) / 8;
	
	if (angleY < -89.0) angleY = -89.0;
	if (angleY > 89.0) angleY = 89.0;
	
	sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);
}

void Camera::Look()
{
	gluLookAt(x, y, z, x - sin(angleX / 180 * pi), y + tan(angleY / 180 * pi), z - cos(angleX / 180 * pi), 0, 1, 0);
}

void Camera::Look(float x, float y, float z)
{
	gluLookAt(this->x, this->y, this->z, x, y, z, 0, 1, 0);
}
