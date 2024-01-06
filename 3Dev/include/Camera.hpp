#pragma once
#include "Utils.hpp"
#include "Node.hpp"
#include "Renderer.hpp"

class Camera : public Node
{
public:
	Camera(sf::Window* window, rp3d::Vector3 pos = rp3d::Vector3::zero(), float speed = 1, float fov = 80, float near = 0.01, float far = 1000.0, Matrices* m = nullptr);
	Camera(sf::Vector2u viewportSize, rp3d::Vector3 pos = rp3d::Vector3::zero(), float speed = 1, float fov = 80, float near = 0.01, float far = 1000.0, Matrices* m = nullptr);

	void Update(bool force = false);

	void Mouse(float sensitivity = 1.0);

	void Look();
	void Look(const rp3d::Vector3& vec);

	void SetViewportSize(sf::Vector2u size);

	void SetTransform(const rp3d::Transform& tr) override;
	void SetPosition(const rp3d::Vector3& vec);
	void SetOrientation(const rp3d::Quaternion& quat);
	void SetSpeed(float speed);
	void SetFOV(float fov);
	void SetNear(float near);
	void SetFar(float far);
	void AlwaysUp(bool a);

	rp3d::Vector3 Move(float time, bool onlyOffset = false);

	rp3d::Transform GetTransform() override;

	rp3d::Vector3 GetPosition(bool world = false);
	rp3d::Quaternion GetOrientation();

	rp3d::Vector2 WorldPositionToScreen(const rp3d::Vector3& world);
	rp3d::Vector3 ScreenPositionToWorld(bool useMousePos, const rp3d::Vector2& screen = { 0, 0 });

	float GetSpeed();
	float GetFOV();
	float GetNear();
	float GetFar();

	Json::Value Serialize();
	void Deserialize(Json::Value data);

private:
	void UpdateMatrix();

	sf::Window* window;
	Matrices* m = Renderer::GetInstance()->GetMatrices();

	float speed, fov, near, far, aspect;
	float angleX = 0, angleY = 0;
	bool alwaysUp = true;

	sf::Vector2u viewportSize = { 0, 0 };

	rp3d::Vector3 pos;
	rp3d::Quaternion orient = rp3d::Quaternion::identity();
};
