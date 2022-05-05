#include <SceneManager.hpp>

void SceneManager::Draw(Framebuffer* fbo)
{
    if(fbo)
    {
        fbo->Bind();
        auto size = fbo->GetSize();
        glViewport(0, 0, size.x, size.y);
    }

    float time = clock.restart().asSeconds();
    std::for_each(pManagers.begin(), pManagers.end(), [&](auto p) { p->Update(time); });
    
    std::for_each(models.begin(), models.end(), [&](auto p) { p->Draw(camera, lights); });
    std::for_each(shapes.begin(), shapes.end(), [&](auto p) { p->Draw(camera, lights); });

    if(skybox)
    {
        glDisable(GL_CULL_FACE);
        skybox->DrawSkybox();
        glEnable(GL_CULL_FACE);
    }
    else if(environment)
    {
        glDisable(GL_CULL_FACE);
        environment->DrawEnvironment();
        glEnable(GL_CULL_FACE);
    }

    if(fbo != nullptr) fbo->Unbind();
}

void SceneManager::AddModel(std::shared_ptr<Model> model)
{
    models.emplace_back(model);
}

void SceneManager::AddShape(std::shared_ptr<Shape> shape)
{
    shapes.emplace_back(shape);
}

void SceneManager::AddPhysicsManager(std::shared_ptr<PhysicsManager> manager)
{
    pManagers.emplace_back(manager);
}

void SceneManager::AddLight(Light* light)
{
    lights.emplace_back(light);
}

void SceneManager::SetMainShader(Shader* shader)
{
    std::for_each(models.begin(), models.end(), [&](auto p) { p->SetShader(shader); });
    std::for_each(shapes.begin(), shapes.end(), [&](auto p) { p->SetShader(shader); });
}

void SceneManager::SetCamera(Camera* camera)
{
    this->camera = camera;
}

void SceneManager::SetSkybox(std::shared_ptr<Shape> skybox)
{
    this->skybox = skybox;
}

void SceneManager::SetEnvironment(std::shared_ptr<Shape> environment)
{
    this->environment = environment;
}
