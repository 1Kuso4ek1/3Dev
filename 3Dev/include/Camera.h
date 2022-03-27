#pragma once
#include "3Dev.h"
#include "Matrices.h"

class Camera {
public:
	Camera(sf::RenderWindow* window, Matrices* m, rp3d::Vector3 pos, float speed, float fov, float near, float far);

	void Update();

	void Move(float time);
	void Mouse();

	void ThirdPerson(float centerx, float centery, float centerz, float dist);

	void Look();
	void Look(const rp3d::Vector3& vec);

	void SetPosition(const rp3d::Vector3& vec);
	void SetOrientation(const rp3d::Quaternion& quat);
	void SetSpeed(const float& speed);
	void SetFOV(const float& fov);
	void SetNear(const float& near);
	void SetFar(const float& far);

	rp3d::Vector3 GetPosition();
	rp3d::Quaternion GetOrientation();

	float GetSpeed();
	float GetFOV();
	float GetNear();
	float GetFar();

private:
	void UpdateMatrix();

	sf::RenderWindow* window;
	Matrices* m;

	float speed, fov, near, far, aspect;
	float angleX = 0, angleY = 0;

	rp3d::Vector3 pos;
	rp3d::Quaternion orient = rp3d::Quaternion::identity();
};
