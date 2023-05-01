#pragma once
#include "Shader.hpp"
#include "Material.hpp"
#include "Camera.hpp"
#include "Matrices.hpp"
#include "Light.hpp"
#include "Mesh.hpp"
#include "PhysicsManager.hpp"
#include "Renderer.hpp"
#include "Node.hpp"
#include "Animation.hpp"

class Model : public Node
{
public:
	Model(bool defaultCubeMesh = false);
	Model(Model* model);
	Model(Shader* shader);
	Model(std::vector<std::shared_ptr<Mesh>> meshes, Shader* shader = nullptr);
	Model(std::string filename, std::vector<Material*> mat, unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs,
		  PhysicsManager* man = nullptr, Shader* shader = nullptr, Matrices* m = nullptr);
	~Model();

	void Load(std::string filename, unsigned int flags = 0);

	void Draw(Node* cam, std::vector<Node*> lights, bool transparencyPass = false) override;
	void DrawSkybox();

	void SetTransform(const rp3d::Transform& transform) override;
	void SetPosition(const rp3d::Vector3& position);
	void SetOrientation(const rp3d::Quaternion& orientation);
	void SetSize(const rp3d::Vector3& size) override;
	void SetMaterial(std::vector<Material*> mat);
	void SetMaterialSlot(Material* mat, unsigned int slot = 0);
	void SetShader(Shader* shader, bool temp = false);
	void SetPhysicsManager(PhysicsManager* man);
	void SetIsDrawable(bool drawable);
	void AddChild(Node* child) override;
	void DefaultPose();

	void CreateRigidBody();

	void Move(const rp3d::Vector3& position);
	void Rotate(const rp3d::Quaternion& orientation);
	void Expand(const rp3d::Vector3& size);

	void CreateBoxShape(unsigned int mesh = 0, rp3d::Transform tr = rp3d::Transform::identity());
	void CreateSphereShape(unsigned int mesh = 0, rp3d::Transform tr = rp3d::Transform::identity());
	void CreateCapsuleShape(unsigned int mesh = 0, rp3d::Transform tr = rp3d::Transform::identity());
	void CreateConcaveShape(unsigned int mesh = 0, rp3d::Transform tr = rp3d::Transform::identity());
	void CreateConvexShape(unsigned int mesh = 0, rp3d::Transform tr = rp3d::Transform::identity());

	int GetMeshesCount();

	bool IsDrawable();

	rp3d::Transform GetTransform() override;
	rp3d::Vector3 GetPosition();
	rp3d::Quaternion GetOrientation();
	rp3d::Vector3 GetSize() override;

	Shader* GetShader();
	rp3d::RigidBody* GetRigidBody() override;

	std::vector<std::shared_ptr<Animation>> GetAnimations();
	std::vector<std::shared_ptr<Bone>> GetBones();
	std::vector<glm::mat4>& GetPose();

	std::string GetFilename();

	std::vector<Material*>& GetMaterial();

	Json::Value Serialize();
	void Deserialize(Json::Value data);

private:
    enum class CollisionShapeType
    {
        Nothing,
        Box,
        Sphere,
        Capsule,
        Concave,
        Convex
    };

	void ProcessNode(aiNode* node, const aiScene* scene);
	void ProcessMesh(aiMesh* mesh, aiNode* node, aiNode* mnode);
	void LoadAnimations(const aiScene* scene);
	void FindBoneNodes(aiNode* node, std::vector<std::shared_ptr<Bone>>& bones);
	void CalculatePose(Bone* bone);

	bool ProcessBone(aiNode* node, std::shared_ptr<Bone>& out);

	bool drawable = true;

	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<Bone>> bones;
	std::vector<std::shared_ptr<Bone>> bonesChildren;
	std::vector<glm::mat4> pose;
	std::unordered_map<std::string, std::pair<int, glm::mat4>> bonemap;

	Matrices* m = Renderer::GetInstance()->GetMatrices();
	Shader* shader = Renderer::GetInstance()->GetShader(Renderer::ShaderType::Main);
	Shader* tempShader = nullptr;

	std::vector<Material*> mat;
	std::vector<std::shared_ptr<Animation>> anims;

	PhysicsManager* man;

	glm::mat4 globalInverseTransform;

	std::vector<std::variant<rp3d::BoxShape*,
							 rp3d::SphereShape*,
							 rp3d::CapsuleShape*,
							 rp3d::ConcaveMeshShape*,
							 rp3d::ConvexMeshShape*>> shapes;
	std::vector<rp3d::Collider*> colliders;
	std::unordered_map<Node*, std::vector<rp3d::Collider*>> childrenColliders;
	rp3d::RigidBody* body = nullptr;
	CollisionShapeType cstype = CollisionShapeType::Nothing;

	rp3d::TriangleVertexArray* triangles = nullptr;
	rp3d::TriangleMesh* tmesh = nullptr;

	rp3d::PolygonVertexArray::PolygonFace* faces = nullptr;
	rp3d::PolygonVertexArray* polygons = nullptr;
	rp3d::PolyhedronMesh* pmesh = nullptr;

	std::string filename;

	rp3d::Vector3 size{ 1, 1, 1 };
	rp3d::Transform transform;
};
