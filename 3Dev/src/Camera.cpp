#include "Camera.hpp"

Camera::Camera(sf::RenderWindow* window, Matrices* m, rp3d::Vector3 pos, float speed, float fov, float near, float far) : window(window), pos(pos), speed(speed), fov(fov), near(near), far(far), m(m)
{
	aspect = (float)window->getSize().x / (float)window->getSize().y;
	UpdateMatrix();
}

void Camera::Update(bool force)
{
	if(((float)window->getSize().x / (float)window->getSize().y) != aspect || force)
	{
		aspect = (float)window->getSize().x / (float)window->getSize().y;
		UpdateMatrix();
	}
}

rp3d::Vector3 Camera::Move(float time)
{
	auto vec = pos;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		pos += (orient * rp3d::Vector3(0, 0, -1)) * speed * time;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		pos -= (orient * rp3d::Vector3(0, 0, -1)) * speed * time;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		pos += (orient * rp3d::Vector3(1, 0, 0)) * speed * time;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		pos -= (orient * rp3d::Vector3(1, 0, 0)) * speed * time;

	return pos - vec;
}

void Camera::Mouse()
{
	sf::Vector2i pixelPos = sf::Mouse::getPosition(*window);
	sf::Vector2f mousexy = window->mapPixelToCoords(pixelPos, sf::View({ (float)window->getSize().x / 2, (float)window->getSize().y / 2 }, { (float)window->getSize().x, (float)window->getSize().y }));
	
	orient = rp3d::Quaternion::fromEulerAngles(0, -glm::radians((mousexy.x - window->getSize().x / 2) / 8), 0) * orient;
	auto tmp = orient * rp3d::Quaternion::fromEulerAngles(-glm::radians((mousexy.y - window->getSize().y / 2) / 8), 0, 0);
	if((tmp * rp3d::Vector3(0, 0, -1)).getAbsoluteVector().y < 0.99) orient = tmp;

	sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2), *window);
}

void Camera::Look()
{	
	rp3d::Vector3 v = orient * rp3d::Vector3(0, 0, -1), tmpv;
	float tmp;
	if(!alwaysUp) orient.getRotationAngleAxis(tmp, tmpv);
	m->GetView() = glm::lookAt(toglm(pos), toglm(pos + v), alwaysUp ? glm::vec3(0.0, 1.0, 0.0) : toglm(tmpv));
}

void Camera::Look(const rp3d::Vector3& vec)
{
	m->GetView() = glm::lookAt(toglm(pos), toglm(vec), glm::vec3(0, 1, 0));
}

void Camera::SetPosition(const rp3d::Vector3& vec)
{
	pos = vec;
}

void Camera::SetOrientation(const rp3d::Quaternion& quat)
{
	orient = quat;
}

void Camera::SetSpeed(const float& speed)
{
	this->speed = speed;
}

void Camera::SetFOV(const float& fov)
{
	this->fov = fov;
	UpdateMatrix();
}

void Camera::SetNear(const float& near)
{
	this->near = near;
	UpdateMatrix();
}

void Camera::SetFar(const float& far)
{
	this->far = far;
	UpdateMatrix();
}

void Camera::AlwaysUp(bool a)
{
	alwaysUp = a;
}

rp3d::Vector3 Camera::GetPosition()
{
	return pos;
}

rp3d::Quaternion Camera::GetOrientation()
{
	return orient;
}

float Camera::GetSpeed()
{
	return speed;
}

float Camera::GetFOV()
{
	return fov;
}

float Camera::GetNear()
{
	return near;
}

float Camera::GetFar()
{
	return far;
}

void Camera::UpdateMatrix()
{
	m->GetProjection() = glm::perspective(glm::radians(fov), aspect, near, far);
}
