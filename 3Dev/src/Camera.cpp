#include "Camera.h"

Camera::Camera(sf::RenderWindow* window, Matrices* m, rp3d::Vector3 pos, float speed, float fov, float near, float far) : window(window), pos(pos), speed(speed), fov(fov), near(near), far(far), m(m)
{
	aspect = (float)window->getSize().x / (float)window->getSize().y;
	UpdateMatrix();
}

void Camera::Update()
{
	if(((float)window->getSize().x / (float)window->getSize().y) != aspect)
	{
		aspect = (float)window->getSize().x / (float)window->getSize().y;
		UpdateMatrix();
	}
}

void Camera::Move(float time)
{
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		pos += (orient * rp3d::Vector3(0, 0, -1)) * speed * time;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		pos -= (orient * rp3d::Vector3(0, 0, -1)) * speed * time;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		pos += (orient * rp3d::Vector3(1, 0, 0)) * speed * time;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		pos -= (orient * rp3d::Vector3(1, 0, 0)) * speed * time;
}

void Camera::ThirdPerson(float centerx, float centery, float centerz, float dist)
{
	float horizontald = dist * cos(angleY * pi / 180);
	float verticald = dist * sin(angleY * pi / 180);
	pos.x = centerx - (horizontald * sin(angleX * pi / 180));
	pos.y = verticald + centery;
	pos.z = centerz - (horizontald * cos(angleX * pi / 180));
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
	rp3d::Vector3 v = orient * rp3d::Vector3(0, 0, -1);
	//glLoadMatrixf(glm::value_ptr(glm::lookAt(toglm(pos), toglm(pos + v), glm::vec3(0, 1, 0))));
	m->GetView() = glm::lookAt(toglm(pos), toglm(pos + v), glm::vec3(0, 1, 0));
}

void Camera::Look(const rp3d::Vector3& vec)
{
	//glLoadMatrixf(glm::value_ptr(glm::lookAt(toglm(pos), toglm(vec), glm::vec3(0, 1, 0))));
	m->GetView() = glm::lookAt(toglm(pos), toglm(vec), glm::vec3(0, 1, 0));
}

rp3d::Vector3 Camera::GetPosition()
{
	return pos;
}

void Camera::SetPosition(const rp3d::Vector3& vec)
{
	pos = vec;
}

rp3d::Quaternion Camera::GetOrientation()
{
	return orient;
}

void Camera::SetOrientation(const rp3d::Quaternion& quat)
{
	orient = quat;
}

void Camera::UpdateMatrix()
{
	//glMatrixMode(GL_PROJECTION);
    //glLoadMatrixf(glm::value_ptr(glm::perspective(glm::radians(fov), aspect, near, far)));
	//glMatrixMode(GL_MODELVIEW);
	m->GetProjection() = glm::perspective(glm::radians(fov), aspect, near, far);
}

void Camera::SetSpeed(const float& speed)
{
	this->speed = speed;
}

float Camera::GetSpeed()
{
	return speed;
}

void Camera::SetFOV(const float& fov)
{
	this->fov = fov;
	UpdateMatrix();
}

float Camera::GetFOV()
{
	return fov;
}

void Camera::SetNear(const float& near)
{
	this->near = near;
	UpdateMatrix();
}

float Camera::GetNear()
{
	return near;
}

void Camera::SetFar(const float& far)
{
	this->far = far;
	UpdateMatrix();
}

float Camera::GetFar()
{
	return far;
}
