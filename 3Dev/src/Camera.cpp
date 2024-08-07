#include "Camera.hpp"

Camera::Camera(sf::Window* window, rp3d::Vector3 pos, float speed, float fov, float near, float far, Matrices* m) : window(window), pos(pos), speed(speed), fov(fov), near(near), far(far)
{
	if(m) this->m = m;
	aspect = (float)window->getSize().x / (float)window->getSize().y;
	viewportSize = window->getSize();
	UpdateMatrix();
}

Camera::Camera(sf::Vector2u viewportSize, rp3d::Vector3 pos, float speed, float fov, float near, float far, Matrices* m) : viewportSize(viewportSize), pos(pos), speed(speed), fov(fov), near(near), far(far)
{
    if(m) this->m = m;
	aspect = (float)viewportSize.x / (float)viewportSize.y;
	UpdateMatrix();
}

void Camera::Update(bool force)
{
	if((((float)viewportSize.x / (float)viewportSize.y) != aspect || force))
	{
		aspect = (float)viewportSize.x / (float)viewportSize.y;
		UpdateMatrix();
	}
}

rp3d::Vector3 Camera::Move(float time, bool onlyOffset, bool noY)
{
	rp3d::Vector3 offset;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
		offset += (orient * rp3d::Vector3(0, 0, -1)) * time;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
		offset -= (orient * rp3d::Vector3(0, 0, -1)) * time;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
		offset += (orient * rp3d::Vector3(1, 0, 0)) * time;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
		offset -= (orient * rp3d::Vector3(1, 0, 0)) * time;

	if(noY)
		offset.y = 0.0;

	offset.normalize();

	if(!onlyOffset)
		pos += offset * speed;

	return offset * speed;
}

void Camera::Mouse(float sensitivity)
{
    if(window)
    {
        sf::Vector2i pixelPos = sf::Mouse::getPosition(*window);
        sf::Vector2f mousexy = sf::Vector2f((float)pixelPos.x, (float)pixelPos.y);

		pitchYaw.x -= glm::radians(((mousexy.y - window->getSize().y / 2) / 8) * sensitivity);
		pitchYaw.y -= glm::radians(((mousexy.x - window->getSize().x / 2) / 8) * sensitivity);

		if(abs(pitchYaw.y) >= M_PI * 2.0)
			pitchYaw.y = 0.0;

		pitchYaw.x = glm::clamp(pitchYaw.x, limits.x, limits.y);
		
		orient = rp3d::Quaternion::fromEulerAngles(rp3d::Vector3(pitchYaw.x, pitchYaw.y, 0.0));

        sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2), *window);
    }
}

void Camera::Look()
{
	auto tr = Node::GetFinalTransform(this) * GetTransform();
	rp3d::Vector3 v = tr.getOrientation() * rp3d::Vector3(0, 0, -1);
	m->GetView() = glm::lookAt(toglm(tr.getPosition()), toglm(tr.getPosition() + v), toglm(upVector));
	m->GetInverseView() = glm::inverse(m->GetView());
}

void Camera::Look(const rp3d::Vector3& vec)
{
	auto tr = Node::GetFinalTransform(this) * GetTransform();
	m->GetView() = glm::lookAt(toglm(tr.getPosition()), toglm(vec), toglm(upVector));
	m->GetInverseView() = glm::inverse(m->GetView());
}

void Camera::SetViewportSize(sf::Vector2u size)
{
	viewportSize = size;
	aspect = size.x / (float)size.y;
	UpdateMatrix();
}

void Camera::SetGuiSize(sf::Vector2u size)
{
	guiSize = size;
}

void Camera::SetTransform(const rp3d::Transform& tr)
{
	pos = tr.getPosition();
	orient = tr.getOrientation();
}

void Camera::SetPosition(const rp3d::Vector3& vec)
{
	pos = vec;
}

void Camera::SetOrientation(const rp3d::Quaternion& quat)
{
	orient = quat;
}

void Camera::SetPitchAndYaw(const rp3d::Vector2& vec)
{
	pitchYaw = vec;
}

void Camera::SetUpVector(const rp3d::Vector3& vec)
{
	upVector = vec;
}

void Camera::SetVerticalLimits(const rp3d::Vector2& vec)
{
	limits = vec;
}

void Camera::SetSpeed(float speed)
{
	this->speed = speed;
}

void Camera::SetFOV(float fov)
{
	this->fov = fov;
	UpdateMatrix();
}

void Camera::SetNear(float near)
{
	this->near = near;
	UpdateMatrix();
}

void Camera::SetFar(float far)
{
	this->far = far;
	UpdateMatrix();
}

rp3d::Transform Camera::GetTransform()
{
	return rp3d::Transform(pos, orient);
}

rp3d::Vector3 Camera::GetPosition(bool world)
{
	if(!world)
		return pos;
	return (Node::GetFinalTransform(this) * GetTransform()).getPosition();
}

rp3d::Quaternion Camera::GetOrientation()
{
	return orient;
}

rp3d::Vector2 Camera::GetPitchAndYaw()
{
	return pitchYaw;
}

rp3d::Vector3 Camera::GetUpVector()
{
	return upVector;
}

rp3d::Vector2 Camera::GetVerticalLimits()
{
	return limits;
}

rp3d::Vector2 Camera::WorldPositionToScreen(const rp3d::Vector3& world, bool useGuiSize)
{
	auto viewSpace = (m->GetView() * glm::vec4(toglm(world), 1.0));
	if(viewSpace.z > 0.0)
		return { -1000, -1000 };

	auto clipSpace = m->GetProjection() * viewSpace;
	if(clipSpace.w == 0.0)
		return { -1000, -1000 };

	auto ndcSpace = glm::vec3(clipSpace) / clipSpace.w;
	auto windowSpace = ((glm::vec2(ndcSpace) + glm::vec2(1.0)) / glm::vec2(2.0)) * glm::vec2(float(useGuiSize ? guiSize.x : viewportSize.x), float(useGuiSize ? guiSize.y : viewportSize.y));

	return { windowSpace.x, (useGuiSize ? guiSize.y : viewportSize.y) - windowSpace.y };
}

rp3d::Vector3 Camera::ScreenPositionToWorld(bool useMousePos, const rp3d::Vector2& screen)
{
	glm::vec2 screenPos;
	if(useMousePos)
	{
		auto pos = sf::Mouse::getPosition(*window);
		screenPos = glm::vec2(float(pos.x), float(pos.y));
	}
	else screenPos = toglm(screen);

	screenPos.x = ((screenPos.x * 2.0) / float(viewportSize.x)) - 1.0;
	screenPos.y = -(((screenPos.y * 2.0) / float(viewportSize.y)) - 1.0);

	glm::vec4 clipPos = glm::vec4(screenPos.x, screenPos.y, -1.0, 1.0);
	clipPos = m->GetInverseProjection() * clipPos;

	glm::vec4 worldPos = glm::normalize(m->GetInverseView() * glm::vec4(clipPos.x, clipPos.y, -1.0, 0.0));

	return { worldPos.x, worldPos.y, worldPos.z };
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
	m->GetInverseProjection() = glm::inverse(m->GetProjection());
}

Json::Value Camera::Serialize()
{
    Json::Value data;
    data["position"]["x"] = pos.x;
    data["position"]["y"] = pos.y;
    data["position"]["z"] = pos.z;

    data["orientation"]["x"] = orient.x;
    data["orientation"]["y"] = orient.y;
    data["orientation"]["z"] = orient.z;
    data["orientation"]["w"] = orient.w;

    data["speed"] = speed;
    data["fov"] = fov;
    data["near"] = near;
    data["far"] = far;

    return data;
}

void Camera::Deserialize(Json::Value data)
{
    pos.x = data["position"]["x"].asFloat();
    pos.y = data["position"]["y"].asFloat();
    pos.z = data["position"]["z"].asFloat();

    orient.x = data["orientation"]["x"].asFloat();
    orient.y = data["orientation"]["y"].asFloat();
    orient.z = data["orientation"]["z"].asFloat();
    orient.w = data["orientation"]["w"].asFloat();

    speed = data["speed"].asFloat();
    fov = data["fov"].asFloat();
    near = data["near"].asFloat();
    far = data["far"].asFloat();

	UpdateMatrix();
}
