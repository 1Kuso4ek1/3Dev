#pragma once
#include <Model.h>
#include <Shape.h>
#include <Light.h>
#include <Camera.h>
#include <Shader.h>
#include <Framebuffer.h>
#include <PhysicsManager.h>

class SceneManager
{
public:
    void Draw(Framebuffer* fbo = nullptr);

    void AddModel(std::shared_ptr<Model> model);
    void AddShape(std::shared_ptr<Shape> shape);
    void AddPhysicsManager(std::shared_ptr<PhysicsManager> manager);
    void AddLight(Light* light);

    void SetMainShader(Shader* shader);
    void SetCamera(Camera* camera);
    void SetSkybox(std::shared_ptr<Shape> skybox);

private:
    sf::Clock clock;

    Camera* camera;

    std::shared_ptr<Shape> skybox;

    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<Shape>> shapes;

    std::vector<std::shared_ptr<PhysicsManager>> pManagers;

    std::vector<Light*> lights;
};
