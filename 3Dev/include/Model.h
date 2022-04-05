#pragma once
#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Camera.h"
#include "Matrices.h"
#include "Light.h"
#include "Mesh.h"
#include "PhysicsManager.h"

struct Keyframe
{
	std::vector<float> posStamps;
	std::vector<float> rotStamps;
	std::vector<float> scaleStamps;

	std::vector<glm::vec3> positions;
	std::vector<glm::quat> rotations;
	std::vector<glm::vec3> scales;
};

struct Animation
{
	enum class State
	{
		Stopped,
		Playing,
		Paused
	};

	float GetTime()
	{
		return time.getElapsedTime().asSeconds() * tps;
	}

	State state = State::Stopped;
	bool repeat = true;

	float duration = 0.0;
	float tps = 1000.0;
	float lastTime = 0.0;

	std::unordered_map<std::string, Keyframe> keyframes;

	sf::Clock time;
};

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

	void CreateBoxShape(int mesh = 0);
	void CreateSphereShape(int mesh = 0);
	void CreateCapsuleShape(int mesh = 0);
	void CreateConcaveShape(int mesh = 0);
	void CreateConvexShape(int mesh = 0);

	void PlayAnimation(int anim = 0);
	void StopAnimation(int anim = 0);
	void PauseAnimation(int anim = 0);
		
	rp3d::Vector3 GetPosition();
	rp3d::Quaternion GetOrientation();
	rp3d::Vector3 GetSize();

	rp3d::RigidBody* GetRigidBody();

	std::vector<Bone>& GetBones(int mesh = 0);
	std::vector<glm::mat4>& GetPose(int mesh = 0);
	
	std::string GetFilename();
	std::string GetTextureFilename();
	
	std::vector<Material>& GetMaterial();

private:
	void ProcessNode(aiNode* node, const aiScene* scene);
	void ProcessMesh(aiMesh* mesh, aiNode* node, aiNode* mnode);
	void LoadAnimations(const aiScene* scene);
	void FindBoneNodes(aiNode* node, std::unordered_map<std::string, std::pair<int, glm::mat4>> boneMap, std::vector<Bone>& bones);
	void CalculatePose(Bone& bone, std::shared_ptr<Mesh>& mesh, glm::mat4 parent = glm::mat4(1.0));

	bool ProcessBone(aiNode* node, std::unordered_map<std::string, std::pair<int, glm::mat4>> boneMap, Bone& out);
	
	std::vector<std::shared_ptr<Mesh>> meshes;
	
	Matrices* m;
	Shader* shader;
	std::vector<Material> mat;
	std::vector<Animation> anims;

	PhysicsManager* man;

	glm::mat4 globalTransform;
	glm::mat4 globalInverseTransform;

	std::vector<rp3d::CollisionShape*> shapes;
	rp3d::RigidBody* body = nullptr;

	rp3d::TriangleVertexArray* triangles = nullptr;
	rp3d::TriangleMesh* tmesh = nullptr;

	rp3d::PolygonVertexArray::PolygonFace* faces = nullptr;
	rp3d::PolygonVertexArray* polygons = nullptr;
	rp3d::PolyhedronMesh* pmesh = nullptr;
	
	std::string filename;

	rp3d::Vector3 size;
	rp3d::Transform transform;
};
