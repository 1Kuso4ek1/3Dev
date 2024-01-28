#pragma once
#include "SceneManager.hpp"

class ShadowManager
{
public:
    ShadowManager(SceneManager* scene, glm::ivec2 size, Shader* depthShader = nullptr);

    bool Update();

private:
    void UpdateShader(Shader* shader);

    SceneManager* scene;

    std::vector<Light*> lights;
    std::vector<std::unique_ptr<Framebuffer>> depthBuffers;

    Shader* depthShader = Renderer::GetInstance()->GetShader(Renderer::ShaderType::Depth);

    glm::ivec2 shadowSize;
};
