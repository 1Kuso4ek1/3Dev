#pragma once
#include "Shader.hpp"
#include "Material.hpp"
#include "Matrices.hpp"
#include "Mesh.hpp"
#include "Renderer.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "PhysicsManager.hpp"

class Shape
{
public:
	Shape(const rp3d::Vector3& size, Material* mat, PhysicsManager* man = nullptr, Shader* shader = nullptr, Matrices* m = nullptr);

	void Draw(Camera* cam, std::vector<Light*> lights);
	void DrawSkybox();

	void SetPosition(const rp3d::Vector3& position);
	void SetOrientation(const rp3d::Quaternion& orientation);
	void SetSize(const rp3d::Vector3& size);
	void SetMaterial(Material* mat);
	void SetShader(Shader* shader);
	
	bool IsTransparent();
	
	Shader* GetShader();
	rp3d::RigidBody* GetRigidBody();
	Material* GetMaterial();
	
	rp3d::Vector3 GetPosition();
	rp3d::Quaternion GetOrientation();
	rp3d::Vector3 GetSize();

private:
    bool transparent = false;
    
	Material* mat;
	Matrices* m = Renderer::GetInstance()->GetMatrices();
	Shader* shader = Renderer::GetInstance()->GetShader(Renderer::ShaderType::Main);

	rp3d::BoxShape* shape = nullptr;
	rp3d::RigidBody* body = nullptr;
	rp3d::Collider* collider = nullptr;

	std::shared_ptr<Mesh> cube;

	rp3d::Vector3 size;
	rp3d::Transform tr;
};
