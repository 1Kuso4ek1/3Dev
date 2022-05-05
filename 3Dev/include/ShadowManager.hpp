#pragma once
#include <SceneManager.hpp>

class ShadowManager
{
public:
    ShadowManager(SceneManager* scene, std::vector<Light*> lights, Shader* mainShader, Shader* depthShader, glm::ivec2 size);

    void Update();

private:
    SceneManager* scene;

    std::vector<Light*> lights;
    std::vector<GLuint> textures;
    std::vector<glm::mat4> lightSpaces;
    std::vector<Framebuffer> depthBuffers;

    Shader* mainShader; 
    Shader* depthShader;

    glm::ivec2 shadowSize;
};
