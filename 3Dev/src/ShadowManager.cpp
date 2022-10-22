#include <ShadowManager.hpp>

ShadowManager::ShadowManager(SceneManager* scene, std::vector<Light*> lights, glm::ivec2 size, Shader* mainShader, Shader* depthShader)
                            : scene(scene), lights(lights), shadowSize(size)
{
    if(mainShader) this->mainShader = mainShader;
    if(depthShader) this->depthShader = depthShader;

    glEnable(GL_CULL_FACE);
    textures.resize(lights.size(), 0);
    lightSpaces.resize(lights.size(), glm::mat4(1.0));
    for(int i = 0; i < lights.size(); i++)
    	depthBuffers.emplace_back(nullptr, size.x, size.y, true);
}

void ShadowManager::Update()
{
    scene->SetMainShader(depthShader);

    glCullFace(GL_FRONT);
   	depthShader->Bind();
    for(int i = 0; i < lights.size(); i++)
    {
        glm::mat4 lprojection = glm::perspective(glm::radians(90.0), 1.0, 0.01, 500.0);
        glm::mat4 lview = glm::lookAt(toglm(lights[i]->GetPosition()),
                                      toglm(lights[i]->GetDirection()),
                                      glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 lspace = lprojection * lview;
        lightSpaces[i] = lspace;

        depthShader->SetUniformMatrix4("light", lspace);

        scene->Draw(&depthBuffers[i]);

        textures[i] = depthBuffers[i].GetTexture(true);
    }

    scene->SetMainShader(mainShader);

    mainShader->Bind();
    for(int i = 0; i < lights.size(); i++)
    {
        mainShader->SetUniformMatrix4("lspace[" + std::to_string(i) + "]", lightSpaces[i]);

        glActiveTexture(GL_TEXTURE9 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        mainShader->SetUniform1i("shadows[" + std::to_string(i) + "].shadowmap", 9 + i);
        mainShader->SetUniform1i("shadows[" + std::to_string(i) + "].isactive", 1);
    }

    glCullFace(GL_BACK);
}
