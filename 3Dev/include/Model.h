#pragma once
#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Camera.h"
#include "Matrices.h"
#include "Light.h"
#include "Mesh.h"
#include "PhysicsManager.h"

class Model
{
public:
	Model(std::string filename, std::vector<Material> mat, Shader* shader, Matrices* m, PhysicsManager* man, unsigned int flags = 0, rp3d::Vector3 position = rp3d::Vector3(0, 0, 0), rp3d::Quaternion orientation = rp3d::Quaternion::identity(), rp3d::Vector3 size = rp3d::Vector3(1, 1, 1));

	void Load(std::string filename, unsigned int flags = 0);

	void Draw(Camera& cam, std::vector<Light> lights);
	
	void SetPosition(rp3d::Vector3 position);
	void SetOrientation(rp3d::Quaternion orientation);
	void SetSize(rp3d::Vector3 size);
	void SetMaterial(std::vector<Material> mat);
	
	void AddPosition(rp3d::Vector3 position);
	void AddRotation(rp3d::Quaternion orientation);
	void AddSize(rp3d::Vector3 size);

	void CreateBoxShape();
	void CreateSphereShape();
	void CreateCapsuleShape();
	void CreateConcaveShape();
	void CreateConvexShape();
		
	rp3d::Vector3 GetPosition();
	rp3d::Quaternion GetOrientation();
	rp3d::Vector3 GetSize();
	
	std::string GetFilename();
	std::string GetTextureFilename();
	
	std::vector<Material> GetMaterial();

private:
	void ProcessNode(aiNode* node, const aiScene* scene);
	void ProcessMesh(aiMesh* mesh);
	
	std::vector<std::shared_ptr<Mesh>> meshes;
	
	Matrices* m;
	Shader* shader;
	std::vector<Material> mat;

	PhysicsManager* man;

	rp3d::CollisionShape* shape = nullptr;
	rp3d::RigidBody* body = nullptr;
	rp3d::Collider* collider = nullptr;

	rp3d::TriangleVertexArray* triangles = nullptr;
	rp3d::TriangleMesh* mesh = nullptr;

	rp3d::PolygonVertexArray::PolygonFace* faces = nullptr;
	rp3d::PolygonVertexArray* polygons = nullptr;
	rp3d::PolyhedronMesh* pmesh = nullptr;
	
	std::string filename;

	rp3d::Vector3 size;
	rp3d::Transform transform;
};
