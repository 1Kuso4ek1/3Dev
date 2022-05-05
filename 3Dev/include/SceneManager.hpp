#pragma once
#include <Model.hpp>
#include <Shape.hpp>
#include <Light.hpp>
#include <Camera.hpp>
#include <Shader.hpp>
#include <Framebuffer.hpp>
#include <PhysicsManager.hpp>

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
    void SetEnvironment(std::shared_ptr<Shape> environment);

private:
    sf::Clock clock;

    Camera* camera;

    std::shared_ptr<Shape> skybox;
    std::shared_ptr<Shape> environment;

    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<Shape>> shapes;

    std::vector<std::shared_ptr<PhysicsManager>> pManagers;

    std::vector<Light*> lights;
};
