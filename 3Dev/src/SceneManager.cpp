#include <SceneManager.hpp>

void SceneManager::Draw(Framebuffer* fbo, Framebuffer* transparency)
{
    if(!fbo) fbo = Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main);
    
    fbo->Bind();
    auto size = fbo->GetSize();
    glViewport(0, 0, size.x, size.y);

    float time = clock.restart().asSeconds();
    std::for_each(pManagers.begin(), pManagers.end(), [&](auto p) { p->Update(time); });

    std::for_each(models.begin(), models.end(), [&](auto p) { p->Draw(camera, lights); });
    std::for_each(shapes.begin(), shapes.end(), [&](auto p) { p->Draw(camera, lights); });

    if(skybox)
    {
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        skybox->DrawSkybox();
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
    }

    fbo->Unbind();

    if(!transparency) transparency = Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Transparency);

    glFrontFace(GL_CW);
    glDisable(GL_CULL_FACE);
    transparency->Bind();
    size = transparency->GetSize();
    glViewport(0, 0, size.x, size.y);

    std::for_each(transparentModels.begin(), transparentModels.end(), [&](auto p) { p->Draw(camera, lights); });
    std::for_each(transparentShapes.begin(), transparentShapes.end(), [&](auto p) { p->Draw(camera, lights); });

    transparency->Unbind();
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
}

void SceneManager::AddObject(std::shared_ptr<Model> model)
{
    if(std::find_if(model->GetMaterial().begin(),
                    model->GetMaterial().end(),
                    [&](auto& a)
                    {
                        return a.Contains(Material::Type::Opacity);
                    }) != model->GetMaterial().end())
        transparentModels.emplace_back(model);
    else models.emplace_back(model);
}

void SceneManager::AddObject(std::shared_ptr<Shape> shape)
{
    if(shape->GetMaterial()->Contains(Material::Type::Opacity))
        transparentShapes.emplace_back(shape);
    else shapes.emplace_back(shape);
}

void SceneManager::AddPhysicsManager(std::shared_ptr<PhysicsManager> manager)
{
    pManagers.emplace_back(manager);
}

void SceneManager::AddLight(Light* light)
{
    lights.emplace_back(light);
}

void SceneManager::RemoveObject(std::shared_ptr<Model> model)
{
    auto it = std::find(models.begin(), models.end(), model);
    if(it != models.end())
        models.erase(it);

    auto it1 = std::find(transparentModels.begin(), transparentModels.end(), model);
    if(it1 != transparentModels.end())
        transparentModels.erase(it1);
}

void SceneManager::RemoveObject(std::shared_ptr<Shape> shape)
{
    auto it = std::find(shapes.begin(), shapes.end(), shape);
    if(it != shapes.end())
        shapes.erase(it);

    auto it1 = std::find(transparentShapes.begin(), transparentShapes.end(), shape);
    if(it1 != transparentShapes.end())
        transparentShapes.erase(it1);
}

void SceneManager::RemovePhysicsManager(std::shared_ptr<PhysicsManager> manager)
{
    auto it = std::find(pManagers.begin(), pManagers.end(), manager);
    if(it != pManagers.end())
        pManagers.erase(it);
}

void SceneManager::RemoveLight(Light* light)
{
    auto it = std::find(lights.begin(), lights.end(), light);
    if(it != lights.end())
        lights.erase(it);
}

void SceneManager::RemoveAllObjects()
{
    models.clear();
    shapes.clear();
    transparentModels.clear();
    transparentShapes.clear();
}

void SceneManager::Save(std::string filename)
{
	std::vector<std::vector<Material>> materials;
    std::vector<Material*> shapeMaterials;
    
    for(auto& i : models)
    {
        if(std::find_if(materials.begin(), materials.end(), [&](auto& a)
            {
                if(a.size() != i->GetMaterial().size()) return false;
                for(int j = 0; j < a.size(); j++)
                {
                    for(int k = 0; k < i->GetMaterial().size(); k++)
                        if(a[j] != i->GetMaterial()[k])
                            return false;
                }
                return true;
            }) == materials.end())
        materials.push_back(i->GetMaterial());
    }
    for(auto& i : shapes)
    {
        if(std::find_if(shapeMaterials.begin(), shapeMaterials.end(), [&](auto& a)
                {
                    return *a == *i->GetMaterial();
                }) == shapeMaterials.end())
            shapeMaterials.push_back(i->GetMaterial());
    }
    for(int i = 0; i < models.size(); i++)
    {
        auto pos = models[i]->GetPosition();
        auto size = models[i]->GetSize();
        auto orient = models[i]->GetOrientation();
        auto modelFilename = models[i]->GetFilename();

        root["models"][i]["filename"] = modelFilename;
        root["models"][i]["material"] = std::find_if(materials.begin(), materials.end(), [&](auto& a)
                                        {
                                            if(a.size() != models[i]->GetMaterial().size()) return false;
                                            for(int j = 0; j < a.size(); j++)
                                            {
                                                for(int k = 0; k < models[i]->GetMaterial().size(); k++)
                                                    if(a[j] != models[i]->GetMaterial()[k])
                                                        return false;
                                            }
                                            return true;
                                        }) - materials.begin();

        root["models"][i]["position"]["x"] = pos.x;
        root["models"][i]["position"]["y"] = pos.y;
        root["models"][i]["position"]["z"] = pos.z;

        root["models"][i]["orientation"]["x"] = orient.x;
        root["models"][i]["orientation"]["y"] = orient.y;
        root["models"][i]["orientation"]["z"] = orient.z;
        root["models"][i]["orientation"]["w"] = orient.w;

        root["models"][i]["size"]["x"] = size.x;
        root["models"][i]["size"]["y"] = size.y;
        root["models"][i]["size"]["z"] = size.z;
    }
    for(int i = 0; i < shapes.size(); i++)
    {
        auto pos = shapes[i]->GetPosition();
        auto size = shapes[i]->GetSize();
        auto orient = shapes[i]->GetOrientation();

        root["shapes"][i]["material"] = std::find_if(shapeMaterials.begin(), shapeMaterials.end(), [&](auto& a)
                                        {
                                            return *a == *(shapes[i]->GetMaterial());
                                        }) - shapeMaterials.begin();

        root["shapes"][i]["position"]["x"] = pos.x;
        root["shapes"][i]["position"]["y"] = pos.y;
        root["shapes"][i]["position"]["z"] = pos.z;

        root["shapes"][i]["orientation"]["x"] = orient.x;
        root["shapes"][i]["orientation"]["y"] = orient.y;
        root["shapes"][i]["orientation"]["z"] = orient.z;
        root["shapes"][i]["orientation"]["w"] = orient.w;

        root["shapes"][i]["size"]["x"] = size.x;
        root["shapes"][i]["size"]["y"] = size.y;
        root["shapes"][i]["size"]["z"] = size.z;
    }
    std::ofstream file(filename);
    file << root.toStyledString();
    file.close();
}

void SceneManager::Load(std::string filename)
{

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

void SceneManager::SetSoundManager(std::shared_ptr<SoundManager> manager)
{
    sManager = manager;
}
