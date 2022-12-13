#pragma once
#include "Model.hpp"
#include "Shape.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Framebuffer.hpp"
#include "PhysicsManager.hpp"
#include "SoundManager.hpp"

class SceneManager
{
public:
    void Draw(Framebuffer* fbo = nullptr, Framebuffer* transparency = nullptr, bool shadows = false);

    void AddObject(std::shared_ptr<Model> model, std::string name = "model", bool checkUniqueness = true);
    void AddObject(std::shared_ptr<Shape> shape, std::string name = "shape", bool checkUniqueness = true);
    void AddMaterial(std::shared_ptr<Material>, std::string name = "material");
    void AddPhysicsManager(std::shared_ptr<PhysicsManager> manager, std::string name = "pmanager");
    void AddLight(Light* light, std::string name = "light");

    void RemoveObject(std::shared_ptr<Model> model);
    void RemoveObject(std::shared_ptr<Shape> shape);
    void RemoveMaterial(std::shared_ptr<Material> material);
    void RemovePhysicsManager(std::shared_ptr<PhysicsManager> manager);
    void RemoveLight(Light* light);

    void RemoveAllObjects();

    void Save(std::string filename, bool relativePaths = false);
    void Load(std::string filename);

    void SaveState();
    void LoadState();

    void SetMainShader(Shader* shader);
    void SetCamera(Camera* camera);
    void SetSkybox(std::shared_ptr<Shape> skybox);
    void SetSoundManager(std::shared_ptr<SoundManager> manager);

    void SetModelName(std::string name, std::string newName);
    void SetShapeName(std::string name, std::string newName);
    void SetMaterialName(std::string name, std::string newName);
    void SetPhysicsManagerName(std::string name, std::string newName);
    void SetLightName(std::string name, std::string newName);

    void UpdatePhysics(bool update);

    std::shared_ptr<Model> GetModel(std::string name);
    std::shared_ptr<Shape> GetShape(std::string name);
    std::shared_ptr<Material> GetMaterial(std::string name);
    std::shared_ptr<PhysicsManager> GetPhysicsManager(std::string name);
    std::shared_ptr<SoundManager> GetSoundManager();

    std::vector<std::shared_ptr<Model>> GetModelGroup(std::string name);
    std::vector<std::shared_ptr<Shape>> GetShapeGroup(std::string name);

    // For angelscript
    Model* GetModelPtr(std::string name);
    Shape* GetShapePtr(std::string name);
    Material* GetMaterialPtr(std::string name);
    PhysicsManager* GetPhysicsManagerPtr(std::string name);
    SoundManager* GetSoundManagerPtr();

    std::vector<Model*> GetModelPtrGroup(std::string name);
    std::vector<Shape*> GetShapePtrGroup(std::string name);

    Camera* GetCamera();
    Light* GetLight(std::string name);

    std::string GetName(std::shared_ptr<Material> mat);
    std::string GetName(Material* mat);

	// @return array of names, 0 - models, 1 - shapes, 2 - materials, 3 - lights, 4 - physics managers
    std::array<std::vector<std::string>, 5> GetNames();

private:
    struct State
    {
        rp3d::Vector3 pos, size;
        rp3d::Quaternion orient;
    };

    void RemoveFromTheGroup(std::string group, std::shared_ptr<Model> model);
    void RemoveFromTheGroup(std::string group, std::shared_ptr<Shape> shape);

    void MoveToTheGroup(std::string from, std::string to, std::shared_ptr<Model> model);
    void MoveToTheGroup(std::string from, std::string to, std::shared_ptr<Shape> shape);

    std::pair<std::string, std::string> ParseName(std::string in);

    sf::Clock clock;

    Camera* camera;

    bool updatePhysics = true;

    std::shared_ptr<Shape> skybox;
    std::shared_ptr<SoundManager> sManager;

    std::unordered_map<std::string, std::vector<std::shared_ptr<Model>>> modelGroups;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Shape>>> shapeGroups;

    std::unordered_map<std::string, std::shared_ptr<Model>> models;
    std::unordered_map<std::string, std::shared_ptr<Shape>> shapes;

    std::unordered_map<std::string, std::shared_ptr<Material>> materials; // for editor and scene saving

    std::unordered_map<std::string, std::shared_ptr<PhysicsManager>> pManagers;

    std::unordered_map<std::string, Light*> lights;
    std::vector<Light*> lightsVector; // for drawing

    std::unordered_map<std::string, State> savedState;

    Json::Value root;
};
