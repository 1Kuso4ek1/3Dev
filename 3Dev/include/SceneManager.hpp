#pragma once
#include "Model.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Framebuffer.hpp"
#include "PhysicsManager.hpp"
#include "SoundManager.hpp"

class SceneManager
{
public:
    void Draw(Framebuffer* fbo = nullptr, Framebuffer* transparency = nullptr, bool updatePhysics = true);

    void AddModel(std::shared_ptr<Model> model, std::string name = "model", bool checkUniqueness = true);
    void AddMaterial(std::shared_ptr<Material>, std::string name = "material");
    void AddLight(Light* light, std::string name = "light");

    void StoreBones(std::shared_ptr<Model> model, Bone* bone = nullptr);
    void RemoveBones(std::shared_ptr<Model> model, Bone* bone = nullptr);

    template<class... Args>
    std::shared_ptr<Model> CreateModel(std::string name, Args&&... args);

    void RemoveModel(std::shared_ptr<Model> model);
    void RemoveMaterial(std::shared_ptr<Material> material);
    void RemoveLight(Light* light);

    void RemoveAllObjects();

    void Save(std::string filename, bool relativePaths = false);
    void Load(std::string filename);

    void SaveState();
    void LoadState();

    void SetMainShader(Shader* shader);
    void SetCamera(Camera* camera);
    void SetSkybox(std::shared_ptr<Model> skybox);
    void SetPhysicsManager(std::shared_ptr<PhysicsManager> manager);
    void SetSoundManager(std::shared_ptr<SoundManager> manager);

    void SetModelName(std::string name, std::string newName);
    void SetMaterialName(std::string name, std::string newName);
    void SetLightName(std::string name, std::string newName);

    void UpdatePhysics(bool update);

    std::string GetLastAdded();

    std::shared_ptr<Model> GetModel(std::string name);
    std::shared_ptr<Material> GetMaterial(std::string name);
    std::shared_ptr<PhysicsManager> GetPhysicsManager();
    std::shared_ptr<SoundManager> GetSoundManager();

    Node* GetNode(std::string name);

    std::string GetModelName(std::shared_ptr<Model> model);
    std::string GetModelName(Model* model);
    std::string GetNodeName(Node* node);
    std::string GetMaterialName(std::shared_ptr<Material> mat);
    std::string GetMaterialName(Material* mat);

    std::vector<std::shared_ptr<Model>> GetModelGroup(std::string name);

    // For angelscript
    Model* GetModelPtr(std::string name);
    Material* GetMaterialPtr(std::string name);
    PhysicsManager* GetPhysicsManagerPtr();
    SoundManager* GetSoundManagerPtr();

    Model* CloneModel(Model* model, bool isTemporary = true, std::string name = "model");

    std::vector<Model*> GetModelPtrGroup(std::string name);

    Camera* GetCamera();
    Light* GetLight(std::string name);
    Bone* GetBone(std::string name);

    std::vector<Light*> GetShadowCastingLights();

	// @return array of names, 0 - models, 1 - materials, 2 - lights, 3 - nodes, 4 - bones
    std::array<std::vector<std::string>, 5> GetNames();

private:
    struct State
    {
        rp3d::Vector3 pos, size;
        rp3d::Quaternion orient;
    };

    void RemoveFromTheGroup(std::string group, std::shared_ptr<Model> model);
    void MoveToTheGroup(std::string from, std::string to, std::shared_ptr<Model> model);

    std::pair<std::string, std::string> ParseName(std::string in);

    sf::Clock clock;

    Camera* camera;

    bool updatePhysics = true;

    std::string lastAdded = "";

    std::shared_ptr<Model> skybox;
    std::shared_ptr<SoundManager> sManager;
    std::shared_ptr<PhysicsManager> pManager;

    std::vector<std::string> temporaryModelCopies;

    std::unordered_map<std::string, std::vector<std::shared_ptr<Model>>> modelGroups;

    std::unordered_map<std::string, Node*> nodes;
    std::unordered_map<std::string, Bone*> bones;
    std::unordered_map<std::string, std::shared_ptr<Model>> models;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials; // for editor and scene saving
    std::unordered_map<std::string, Light*> lights;

    std::unordered_map<std::string, std::shared_ptr<Animation>> animations;

    std::vector<Node*> lightsVector; // for drawing

    std::unordered_map<std::string, State> savedState;

    Json::Value root;
};
