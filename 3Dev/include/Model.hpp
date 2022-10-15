#pragma once
#include "Shader.hpp"
#include "Material.hpp"
#include "Camera.hpp"
#include "Matrices.hpp"
#include "Light.hpp"
#include "Mesh.hpp"
#include "PhysicsManager.hpp"
#include "Renderer.hpp"

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
	Model(Shader* shader = nullptr);
	Model(std::vector<std::shared_ptr<Mesh>> meshes, Shader* shader = nullptr);
	Model(std::string filename, std::vector<Material*> mat, unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs,
		  PhysicsManager* man = nullptr, Shader* shader = nullptr, Matrices* m = nullptr);

	void Load(std::string filename, unsigned int flags = 0);

	void Draw(Camera* cam, std::vector<Light*> lights);

	void SetPosition(const rp3d::Vector3& position);
	void SetOrientation(const rp3d::Quaternion& orientation);
	void SetSize(const rp3d::Vector3& size);
	void SetMaterial(std::vector<Material*> mat);
	void SetShader(Shader* shader);
	void SetPhysicsManager(PhysicsManager* man);

	void CreateRigidBody();

	void Move(const rp3d::Vector3& position);
	void Rotate(const rp3d::Quaternion& orientation);
	void Expand(const rp3d::Vector3& size);

	void CreateBoxShape(int mesh = 0, rp3d::Transform tr = rp3d::Transform::identity());
	void CreateSphereShape(int mesh = 0, rp3d::Transform tr = rp3d::Transform::identity());
	void CreateCapsuleShape(int mesh = 0, rp3d::Transform tr = rp3d::Transform::identity());
	void CreateConcaveShape(int mesh = 0, rp3d::Transform tr = rp3d::Transform::identity());
	void CreateConvexShape(int mesh = 0, rp3d::Transform tr = rp3d::Transform::identity());

	void PlayAnimation(int anim = 0);
	void StopAnimation(int anim = 0);
	void PauseAnimation(int anim = 0);

	void AutoUpdateAnimation(bool update = true);

	void UpdateAnimation();

	void CheckOpacity();

	int GetMeshesCount();
	int GetAnimationsCount();

	bool IsTransparent();

	rp3d::Vector3 GetPosition();
	rp3d::Quaternion GetOrientation();
	rp3d::Vector3 GetSize();

	Animation::State GetAnimationState(int anim = 0);

	Shader* GetShader();
	rp3d::RigidBody* GetRigidBody();

	std::vector<Bone>& GetBones(int mesh = 0);
	std::vector<glm::mat4>& GetPose(int mesh = 0);

	std::string GetFilename();

	std::vector<Material*>& GetMaterial();

	Json::Value Serialize();
	void Deserialize(Json::Value data);

private:
	void ProcessNode(aiNode* node, const aiScene* scene);
	void ProcessMesh(aiMesh* mesh, aiNode* node, aiNode* mnode);
	void LoadAnimations(const aiScene* scene);
	void FindBoneNodes(aiNode* node, std::unordered_map<std::string, std::pair<int, glm::mat4>> boneMap, std::vector<Bone>& bones);
	void CalculatePose(Bone& bone, std::shared_ptr<Mesh>& mesh, glm::mat4 parent = glm::mat4(1.0));

	bool ProcessBone(aiNode* node, std::unordered_map<std::string, std::pair<int, glm::mat4>> boneMap, Bone& out);

	bool autoUpdateAnimation = true;
	bool transparent = false;

	std::vector<std::shared_ptr<Mesh>> meshes;

	Matrices* m = Renderer::GetInstance()->GetMatrices();
	Shader* shader = Renderer::GetInstance()->GetShader(Renderer::ShaderType::Main);

	std::vector<Material*> mat;
	std::vector<Animation> anims;

	PhysicsManager* man;

	glm::mat4 globalInverseTransform;

	std::vector<rp3d::CollisionShape*> shapes;
	std::vector<rp3d::Collider*> colliders;
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
