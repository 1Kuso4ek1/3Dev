#pragma once
#include "Utils.hpp"
#include "Matrices.hpp"

class Camera {
public:
	Camera(sf::Window* window, Matrices* m, rp3d::Vector3 pos, float speed, float fov, float near, float far);

	void Update(bool force = false);

	rp3d::Vector3 Move(float time);
	void Mouse();

	void Look();
	void Look(const rp3d::Vector3& vec);

	void SetPosition(const rp3d::Vector3& vec);
	void SetOrientation(const rp3d::Quaternion& quat);
	void SetSpeed(const float& speed);
	void SetFOV(const float& fov);
	void SetNear(const float& near);
	void SetFar(const float& far);
	void AlwaysUp(bool a);

	rp3d::Vector3 GetPosition();
	rp3d::Quaternion GetOrientation();

	float GetSpeed();
	float GetFOV();
	float GetNear();
	float GetFar();

private:
	void UpdateMatrix();

	sf::Window* window;
	Matrices* m;

	float speed, fov, near, far, aspect;
	float angleX = 0, angleY = 0;
	bool alwaysUp = true;

	rp3d::Vector3 pos;
	rp3d::Quaternion orient = rp3d::Quaternion::identity();
};
