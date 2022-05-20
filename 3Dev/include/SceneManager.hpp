#pragma once
#include <Model.hpp>
#include <Shape.hpp>
#include <Light.hpp>
#include <Camera.hpp>
#include <Shader.hpp>
#include <Framebuffer.hpp>
#include <PhysicsManager.hpp>

#include <json/json.h>

class SceneManager
{
public:
    void Draw(Framebuffer* fbo = nullptr, Framebuffer* transparency = nullptr);

    void AddObject(std::shared_ptr<Model> model);
    void AddObject(std::shared_ptr<Shape> shape);
    void AddPhysicsManager(std::shared_ptr<PhysicsManager> manager);
    void AddLight(Light* light);

    void RemoveObject(std::shared_ptr<Model> model);
    void RemoveObject(std::shared_ptr<Shape> shape);
    void RemovePhysicsManager(std::shared_ptr<PhysicsManager> manager);
    void RemoveLight(Light* light);

    void RemoveAllObjects();

    void Save(std::string filename);
    void Load(std::string filename);

    void SetMainShader(Shader* shader);
    void SetCamera(Camera* camera);
    void SetSkybox(std::shared_ptr<Shape> skybox);

private:
    sf::Clock clock;

    Camera* camera;

    std::shared_ptr<Shape> skybox;

    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<Shape>> shapes;

    std::vector<std::shared_ptr<Model>> transparentModels;
    std::vector<std::shared_ptr<Shape>> transparentShapes;

    std::vector<std::shared_ptr<PhysicsManager>> pManagers;

    std::vector<Light*> lights;

    Json::Value root;
};
