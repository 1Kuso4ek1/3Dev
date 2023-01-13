#pragma once
#include "SceneManager.hpp"

class ShadowManager
{
public:
    ShadowManager(SceneManager* scene, glm::ivec2 size, Shader* mainShader = nullptr, Shader* depthShader = nullptr);

    void Update();

private:
    SceneManager* scene;

    GLuint texture;
    std::vector<Light*> lights;
    std::unique_ptr<Framebuffer> depthBuffer;

    Shader* mainShader = Renderer::GetInstance()->GetShader(Renderer::ShaderType::Main);
    Shader* depthShader = Renderer::GetInstance()->GetShader(Renderer::ShaderType::Depth);

    glm::ivec2 shadowSize;
};
