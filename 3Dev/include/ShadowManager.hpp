#pragma once
#include "SceneManager.hpp"

class ShadowManager
{
public:
    ShadowManager(SceneManager* scene, glm::ivec2 size, Shader* mainShader = nullptr, Shader* depthShader = nullptr);

    void Update();

private:
    SceneManager* scene;

    std::vector<Light*> lights;
    std::vector<GLuint> textures;
    std::vector<glm::mat4> lightSpaces;
    std::vector<Framebuffer> depthBuffers;

    Shader* mainShader = Renderer::GetInstance()->GetShader(Renderer::ShaderType::Main);
    Shader* depthShader = Renderer::GetInstance()->GetShader(Renderer::ShaderType::Depth);

    glm::ivec2 shadowSize;
};
