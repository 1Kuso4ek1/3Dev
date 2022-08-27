#pragma once
#include "Model.hpp"
#include "Shape.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Framebuffer.hpp"
#include "PhysicsManager.hpp"
#include "SoundManager.hpp"

#include <json/json.h>

class SceneManager
{
public:
    void Draw(Framebuffer* fbo = nullptr, Framebuffer* transparency = nullptr);

    void AddObject(std::shared_ptr<Model> model, std::string name = "model");
    void AddObject(std::shared_ptr<Shape> shape, std::string name = "shape");
    void AddMaterial(std::shared_ptr<Material>, std::string name = "material");
    void AddPhysicsManager(std::shared_ptr<PhysicsManager> manager);
    void AddLight(Light* light, std::string name = "light");

    void RemoveObject(std::shared_ptr<Model> model);
    void RemoveObject(std::shared_ptr<Shape> shape);
    void RemoveMaterial(std::shared_ptr<Material> material);
    void RemovePhysicsManager(std::shared_ptr<PhysicsManager> manager);
    void RemoveLight(Light* light);

    void RemoveAllObjects();

    void Save(std::string filename);
    void Load(std::string filename);

    void SetMainShader(Shader* shader);
    void SetCamera(Camera* camera);
    void SetSkybox(std::shared_ptr<Shape> skybox);
    void SetSoundManager(std::shared_ptr<SoundManager> manager);
    
    void SetModelName(std::string name, std::string newName);
    void SetShapeName(std::string name, std::string newName);
    void SetMaterialName(std::string name, std::string newName);
    void SetLightName(std::string name, std::string newName);

    std::shared_ptr<Model> GetModel(std::string name);
    std::shared_ptr<Shape> GetShape(std::string name);
    std::shared_ptr<Material> GetMaterial(std::string name);
    Light* GetLight(std::string name);

    std::string GetName(std::shared_ptr<Material> mat);
    std::string GetName(Material* mat);

	// @return array of names, 0 - models, 1 - shapes, 2 - materials, 3 - lights
    std::array<std::vector<std::string>, 4> GetNames();

private:
    sf::Clock clock;

    Camera* camera;

    std::shared_ptr<Shape> skybox;
    std::shared_ptr<SoundManager> sManager;

    std::unordered_map<std::string, std::shared_ptr<Model>> models;
    std::unordered_map<std::string, std::shared_ptr<Shape>> shapes;

    std::unordered_map<std::string, std::shared_ptr<Material>> materials; // for editor and scene saving

    std::vector<std::shared_ptr<PhysicsManager>> pManagers;

    std::unordered_map<std::string, Light*> lights;
    std::vector<Light*> lightsVector; // for drawing

    Json::Value root;
};
