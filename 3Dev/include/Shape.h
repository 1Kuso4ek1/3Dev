#pragma once
#include "3Dev.h"
#include "Shader.h"
#include "Material.h"
#include "Matrices.h"
#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "PhysicsManager.h"

class Shape {
public:
	Shape(const rp3d::Vector3& size, Material* mat, Shader* shader, Matrices* m, PhysicsManager* man);
	~Shape();

	void Draw(Camera* cam, std::vector<Light*> lights);
	void DrawSkybox();
	void DrawEnvironment();

	void SetPosition(const rp3d::Vector3& position);
	void SetOrientation(const rp3d::Quaternion& orientation);
	void SetSize(const rp3d::Vector3& size);
	void SetMaterial(Material* mat);
	void SetShader(Shader* shader);

	rp3d::RigidBody* GetRigidBody();
	
	rp3d::Vector3 GetPosition();
	rp3d::Quaternion GetOrientation();
	rp3d::Vector3 GetSize();

private:
	Material* mat;
	Matrices* m;
	Shader* shader;

	rp3d::BoxShape* shape = nullptr;
	rp3d::RigidBody* body = nullptr;
	rp3d::Collider* collider = nullptr;

	float data[192] = 
	{
		1, 1, -1, 0, 1, 0, 0, 0,
		-1, 1, -1, 0, 1, 0, 1, 0,
		-1, 1, 1, 0, 1, 0, 1, 1,
		1, 1, 1, 0, 1, 0, 0, 1,
		1, -1, 1, 0, 0, 1, 0, 0,
		1, 1, 1, 0, 0, 1, 1, 0,
		-1, 1, 1, 0, 0, 1, 1, 1,
		-1, -1, 1, 0, 0, 1, 0, 1,
		-1, -1, 1, -1, 0, 0, 0, 0,
		-1, 1, 1, -1, 0, 0, 1, 0,
		-1, 1, -1, -1, 0, 0, 1, 1,
		-1, -1, -1, -1, 0, 0, 0, 1,
		-1, -1, -1, 0, -1, 0, 0, 0,
		1, -1, -1, 0, -1, 0, 1, 0,
		1, -1, 1, 0, -1, 0, 1, 1,
		-1, -1, 1, 0, -1, 0, 0, 1,
		1, -1, -1, 1, 0, 0, 0, 0,
		1, 1, -1, 1, 0, 0, 1, 0,
		1, 1, 1, 1, 0, 0, 1, 1,
		1, -1, 1, 1, 0, 0, 0, 1,
		-1, -1, -1, 0, 0, -1, 0, 0,
		-1, 1, -1, 0, 0, -1, 1, 0,
		1, 1, -1, 0, 0, -1, 1, 1,
		1, -1, -1, 0, 0, -1, 0, 1
	};

	GLuint indices[36] = 
	{
		0, 1, 2, 0, 2, 3, 4, 5, 6,
		4, 6, 7, 8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15, 16, 17, 18,
		16, 18, 19, 20, 21, 22, 20, 22, 23
	};

	GLuint vao, vbo, ebo;

	rp3d::Vector3 size;
	rp3d::Transform tr;
};
