#include <ShadowManager.hpp>

ShadowManager::ShadowManager(SceneManager* scene, glm::ivec2 size, Shader* mainShader, Shader* depthShader)
                            : scene(scene), lights(scene->GetShadowCastingLights()), shadowSize(size)
{
    if(mainShader) this->mainShader = mainShader;
    if(depthShader) this->depthShader = depthShader;

    glEnable(GL_CULL_FACE);
    //glPolygonOffset(-1, 0.7);

    depthBuffer = std::make_unique<Framebuffer>(nullptr, size.x, size.y, true, GL_LINEAR, GL_CLAMP_TO_BORDER);
}

void ShadowManager::Update()
{
    if(scene->GetShadowCastingLights().size() != lights.size())
        lights = scene->GetShadowCastingLights();

    if(lights.empty())
        return;

    scene->SetMainShader(depthShader, true);

    //glEnable(GL_POLYGON_OFFSET_FILL);
    glCullFace(GL_FRONT);
   	depthShader->Bind();

    Light* closestLight = lights[0];
    for(auto i : lights)
    {
        if((i->GetPosition() - scene->GetCamera()->GetPosition(true)).length() <
           (closestLight->GetPosition() - scene->GetCamera()->GetPosition(true)).length())
            closestLight = i;
    }
    
    if(closestLight->GetParent())
        closestLight->CalcLightSpaceMatrix();
    depthShader->SetUniformMatrix4("light", closestLight->GetLightSpaceMatrix());

    scene->Draw(depthBuffer.get(), nullptr);

    texture = depthBuffer->GetTexture(true);

    mainShader->Bind();

    mainShader->SetUniformMatrix4("lspace", closestLight->GetLightSpaceMatrix());
    auto pos = closestLight->GetPosition();

    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, texture);
    mainShader->SetUniform3f("shadow.sourcepos", pos.x, pos.y, pos.z);
    mainShader->SetUniform1i("shadow.shadowmap", 9);
    mainShader->SetUniform1i("shadow.perspective", closestLight->IsCastingPerspectiveShadows());

    glCullFace(GL_BACK);
    //glDisable(GL_POLYGON_OFFSET_FILL);
}
