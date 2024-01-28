#include <ShadowManager.hpp>

ShadowManager::ShadowManager(SceneManager* scene, glm::ivec2 size, Shader* depthShader)
                            : scene(scene), lights(scene->GetShadowCastingLights()), shadowSize(size)
{
    if(depthShader) this->depthShader = depthShader;

    glEnable(GL_CULL_FACE);
    glPolygonOffset(-1.0, 0.0);

    for(int i = 0; i < (lights.size() > 8 ? 8 : lights.size()); i++)
        depthBuffers.emplace_back(std::make_unique<Framebuffer>(nullptr, shadowSize.x, shadowSize.y, true, true, 1, GL_LINEAR, GL_CLAMP_TO_BORDER));
}

bool ShadowManager::Update()
{
    lights = scene->GetShadowCastingLights();

    if(lights.empty())
        return true;
        
    if(lights.size() > depthBuffers.size())
        for(int i = 0; i < (lights.size() > 8 ? 8 - depthBuffers.size() : (lights.size() - depthBuffers.size())); i++)
            depthBuffers.emplace_back(std::make_unique<Framebuffer>(nullptr, shadowSize.x, shadowSize.y, true, true, 1, GL_LINEAR, GL_CLAMP_TO_BORDER));
    else if(lights.size() < depthBuffers.size())
        depthBuffers.resize(lights.size());

    glEnable(GL_POLYGON_OFFSET_FILL);
    glCullFace(GL_FRONT);

    std::sort(lights.begin(), lights.end(), [&](auto l, auto l1)
    {
        return (l->GetPosition() - scene->GetCamera()->GetPosition(true)).length() <
               (l1->GetPosition() - scene->GetCamera()->GetPosition(true)).length();
    });
    
    for(int i = 0; i < (lights.size() > 8 ? 8 : lights.size()); i++)
    {
        scene->SetMainShader(depthShader, true);

        depthShader->Bind();
        if(lights[i]->GetParent())
            lights[i]->CalcLightSpaceMatrix();
        depthShader->SetUniformMatrix4("light", lights[i]->GetLightSpaceMatrix());

        scene->Draw(depthBuffers[i].get(), nullptr, i == 0, true);
    }

    UpdateShader(Renderer::GetInstance()->GetShader(Renderer::ShaderType::LightingPass));
    UpdateShader(Renderer::GetInstance()->GetShader(Renderer::ShaderType::Forward));

    glCullFace(GL_BACK);
    glDisable(GL_POLYGON_OFFSET_FILL);

    return false;
}

void ShadowManager::UpdateShader(Shader* shader)
{
    shader->Bind();
    for(int i = 0; i < 8; i++)
	{
		glActiveTexture(GL_TEXTURE22 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
        shader->SetUniform1i("shadows[" + std::to_string(i) + "].isactive", false);
		shader->SetUniform1i("shadows[" + std::to_string(i) + "].shadowMap", 22 + i);
	}

    for(int i = 0; i < (lights.size() > 8 ? 8 : lights.size()); i++)
    {
        shader->SetUniformMatrix4("lspace[" + std::to_string(i) + "]", lights[i]->GetLightSpaceMatrix());
        auto pos = lights[i]->GetPosition(true);

        glActiveTexture(GL_TEXTURE22 + i);
        glBindTexture(GL_TEXTURE_2D, depthBuffers[i]->GetTexture(true));
        shader->SetUniform3f("shadows[" + std::to_string(i) + "].sourcepos", pos.x, pos.y, pos.z);
        shader->SetUniform1i("shadows[" + std::to_string(i) + "].isactive", true);
        shader->SetUniform1i("shadows[" + std::to_string(i) + "].shadowMap", 22 + i);
    }
}
