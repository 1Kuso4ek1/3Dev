#include <SceneManager.hpp>

void SceneManager::Draw(Framebuffer* fbo, Framebuffer* transparency)
{
    if(!fbo) fbo = Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main);
    
    fbo->Bind();
    auto size = fbo->GetSize();
    glViewport(0, 0, size.x, size.y);

    float time = clock.restart().asSeconds();
    std::for_each(pManagers.begin(), pManagers.end(), [&](auto p) { p->Update(time); });

	// needed for materials without textures to render correctly in some cases
    for(int i = 0; i < 8; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

    std::for_each(models.begin(), models.end(), [&](auto p) { p.second->Draw(camera, lightsVector); });
    std::for_each(shapes.begin(), shapes.end(), [&](auto p) { p.second->Draw(camera, lightsVector); });

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

    std::for_each(models.begin(), models.end(), [&](auto p) { if(p.second->IsTransparent()) p.second->Draw(camera, lightsVector); });
    std::for_each(shapes.begin(), shapes.end(), [&](auto p) { if(p.second->IsTransparent()) p.second->Draw(camera, lightsVector); });

    transparency->Unbind();
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
}

void SceneManager::AddObject(std::shared_ptr<Model> model, std::string name)
{
    int nameCount = std::count_if(models.begin(), models.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });
                    
    models[name + (nameCount ? std::to_string(nameCount) : "")] = model;
}

void SceneManager::AddObject(std::shared_ptr<Shape> shape, std::string name)
{
    int nameCount = std::count_if(shapes.begin(), shapes.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });
                    
    shapes[name + (nameCount ? std::to_string(nameCount) : "")] = shape;
}

void SceneManager::AddMaterial(std::shared_ptr<Material> material, std::string name)
{
    int nameCount = std::count_if(materials.begin(), materials.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });
                    
    materials[name + (nameCount ? std::to_string(nameCount) : "")] = material;
}

void SceneManager::AddPhysicsManager(std::shared_ptr<PhysicsManager> manager)
{
    pManagers.emplace_back(manager);
}

void SceneManager::AddLight(Light* light, std::string name)
{
    int nameCount = std::count_if(lights.begin(), lights.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });
                    
    lights[name + (nameCount ? std::to_string(nameCount) : "")] = light;
}

void SceneManager::RemoveObject(std::shared_ptr<Model> model)
{
    auto it = std::find_if(models.begin(), models.end(), [&](auto& p) { return p.second == model; });
    if(it != models.end()) models.erase(it);
}

void SceneManager::RemoveObject(std::shared_ptr<Shape> shape)
{
    auto it = std::find_if(shapes.begin(), shapes.end(), [&](auto& p) { return p.second == shape; });
    if(it != shapes.end()) shapes.erase(it);
}

void SceneManager::RemoveMaterial(std::shared_ptr<Material> material)
{
    auto it = std::find_if(materials.begin(), materials.end(), [&](auto& p) { return p.second == material; });
    if(it != materials.end())
        materials.erase(it);
}

void SceneManager::RemovePhysicsManager(std::shared_ptr<PhysicsManager> manager)
{
    auto it = std::find(pManagers.begin(), pManagers.end(), manager);
    if(it != pManagers.end()) pManagers.erase(it);
}

void SceneManager::RemoveLight(Light* light)
{
    auto it = std::find_if(lights.begin(), lights.end(), [&](auto& p) { return p.second == light; });
    if(it != lights.end()) 
    {
        lights.erase(it);
        lightsVector.erase(std::find(lightsVector.begin(), lightsVector.end(), it->second));
    }
}

void SceneManager::RemoveAllObjects()
{
    models.clear();
    shapes.clear();
}

void SceneManager::Save(std::string filename)
{
	
}

void SceneManager::Load(std::string filename)
{

}

void SceneManager::SetMainShader(Shader* shader)
{
    std::for_each(models.begin(), models.end(), [&](auto p) { p.second->SetShader(shader); });
    std::for_each(shapes.begin(), shapes.end(), [&](auto p) { p.second->SetShader(shader); });
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

std::shared_ptr<Model> SceneManager::GetModel(std::string name)
{
    if(models.find(name) != models.end())
        return models[name];
    Log::Write("Could not find a model with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

std::shared_ptr<Shape> SceneManager::GetShape(std::string name)
{
    if(shapes.find(name) != shapes.end())
        return shapes[name];
    Log::Write("Could not find a shape with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

std::shared_ptr<Material> SceneManager::GetMaterial(std::string name)
{
    if(materials.find(name) != materials.end())
        return materials[name];
    Log::Write("Could not find a material with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

Light* SceneManager::GetLight(std::string name)
{
    if(lights.find(name) != lights.end())
        return lights[name];
    Log::Write("Could not find a light with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

void SceneManager::SetModelName(std::string name, std::string newName)
{
	auto it = models.find(name);
	if(it != models.end())
	{
		auto n = models.extract(it);
		n.key() = newName;
		models.insert(std::move(n));
	}
    else
        Log::Write("Could not find a model with name \"" + name + "\"", Log::Type::Warning);
}

void SceneManager::SetShapeName(std::string name, std::string newName)
{
	auto it = shapes.find(name);
	if(it != shapes.end())
	{
		auto n = shapes.extract(it);
		n.key() = newName;
		shapes.insert(std::move(n));
	}
    else
        Log::Write("Could not find a shape with name \"" + name + "\"", Log::Type::Warning);
}

void SceneManager::SetMaterialName(std::string name, std::string newName)
{
	auto it = materials.find(name);
	if(it != materials.end())
	{
		auto n = materials.extract(it);
		n.key() = newName;
		materials.insert(std::move(n));
	}
    else
        Log::Write("Could not find a material with name \"" + name + "\"", Log::Type::Warning);
}

void SceneManager::SetLightName(std::string name, std::string newName)
{
	auto it = lights.find(name);
	if(it != lights.end())
	{
		auto n = lights.extract(it);
		n.key() = newName;
		lights.insert(std::move(n));
	}
    else
        Log::Write("Could not find a light with name \"" + name + "\"", Log::Type::Warning);
}

std::array<std::vector<std::string>, 4> SceneManager::GetNames()
{
	std::array<std::vector<std::string>, 4> ret;
    std::vector<std::string> tmp;
    for(auto& i : models)
        tmp.push_back(i.first);
    ret[0] = tmp; tmp.clear();
    for(auto& i : shapes)
        tmp.push_back(i.first);
    ret[1] = tmp; tmp.clear();
    for(auto& i : materials)
        tmp.push_back(i.first);
    ret[2] = tmp; tmp.clear();
    for(auto& i : lights)
        tmp.push_back(i.first);
    ret[3] = tmp;
    return ret;
}

std::string SceneManager::GetName(std::shared_ptr<Material> mat)
{
    return std::find_if(materials.begin(), materials.end(), [&](auto m)
            {
                return m.second == mat;
            })->first;
}

std::string SceneManager::GetName(Material* mat)
{
    return std::find_if(materials.begin(), materials.end(), [&](auto m)
            {
                return m.second.get() == mat;
            })->first;
}
