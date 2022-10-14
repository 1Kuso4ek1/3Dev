#include <SceneManager.hpp>

void SceneManager::Draw(Framebuffer* fbo, Framebuffer* transparency)
{
    if(!fbo) fbo = Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main);

    fbo->Bind();
    auto size = fbo->GetSize();
    glViewport(0, 0, size.x, size.y);

    float time = clock.restart().asSeconds();
    std::for_each(pManagers.begin(), pManagers.end(), [&](auto p) { p.second->Update(time); });

    // needed for materials without textures to render correctly in some cases
    for(int i = 0; i < 8; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

    std::for_each(models.begin(), models.end(), [&](auto p) { if(!p.second->IsTransparent()) p.second->Draw(camera, lightsVector); });
    std::for_each(shapes.begin(), shapes.end(), [&](auto p) { if(!p.second->IsTransparent()) p.second->Draw(camera, lightsVector); });

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

void SceneManager::AddPhysicsManager(std::shared_ptr<PhysicsManager> manager, std::string name)
{
    int nameCount = std::count_if(pManagers.begin(), pManagers.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });

    pManagers[name + (nameCount ? std::to_string(nameCount) : "")] = manager;
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
    auto it = std::find_if(pManagers.begin(), pManagers.end(), [&](auto& p) { return p.second == manager; });
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
    Json::Value data;
    int counter = 0;

    for(auto& i : materials)
    {
        data["materials"][counter] = i.second->Serialize();
        data["materials"][counter]["name"] = i.first;
        counter++;
    }
    counter = 0;

	for(auto& i : shapes)
    {
        data["objects"]["shapes"][counter] = i.second->Serialize();
        data["objects"]["shapes"][counter]["name"] = i.first;
        std::string materialName = GetName(i.second->GetMaterial());/*std::find_if(materials.begin(), materials.end(), [&](auto& a)
                                                {
                                                    return *a.second.get() == *i.second->GetMaterial();
                                                })->first*/;
        data["objects"]["shapes"][counter]["material"] = materialName;
        counter++;
    }
    counter = 0;

    for(auto& i : models)
    {
        data["objects"]["models"][counter] = i.second->Serialize();
        data["objects"]["models"][counter]["name"] = i.first;
        std::vector<std::string> materialNames;
        auto mat = i.second->GetMaterial();
        for(auto& i : mat)
            materialNames.push_back(GetName(i));
        for(int i = 0; i < materialNames.size(); i++)
            data["objects"]["models"][counter]["material"][i]["name"] = materialNames[i];
        counter++;
    }

    std::ofstream file(filename);
    file << data.toStyledString();
    file.close();
}

void SceneManager::Load(std::string filename)
{
    Json::Value data;
    Json::CharReaderBuilder rbuilder;

    std::ifstream file(filename);

    std::string errors;
    if(!Json::parseFromStream(rbuilder, file, &data, &errors))
    {
        Log::Write("Json parsing failed: " + errors, Log::Type::Error);
        return;
    }

    /*rp3d::PhysicsWorld::WorldSettings settings;
    pManagers.emplace_back(std::make_shared<PhysicsManager>(settings));*/

    //materials["default"] = std::make_shared<Material>();


    int counter = 0;
    while(!data["materials"][counter].empty())
    {
        auto name = data["materials"][counter]["name"].asString();
        materials[name] = std::make_shared<Material>();
        materials[name]->Deserialize(data["materials"][counter]);
        counter++;
    }

    counter = 0;
    while(!data["objects"]["shapes"][counter].empty())
    {
        auto name = data["objects"]["shapes"][counter]["name"].asString();
        auto material = materials[data["objects"]["shapes"][counter]["material"].asString()].get();
        shapes[name] = std::make_shared<Shape>(rp3d::Vector3::zero(), material, pManagers.begin()->second.get());
        shapes[name]->Deserialize(data["objects"]["shapes"][counter]);
        counter++;
    }

    counter = 0;
    while(!data["objects"]["models"][counter].empty())
    {
        auto name = data["objects"]["models"][counter]["name"].asString();

        //auto material = materials[data["objects"]["models"][counter]["material"].asString()].get();
        std::vector<Material*> material;
        for(auto& i : data["objects"]["models"][counter]["material"])
            material.push_back(materials[i["name"].asString()].get());

        models[name] = std::make_shared<Model>(data["objects"]["models"][counter]["filename"].asString(), material,
                                               aiProcess_Triangulate | aiProcess_FlipUVs, pManagers.begin()->second.get());
        models[name]->Deserialize(data["objects"]["models"][counter]);
        counter++;
    }
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

std::shared_ptr<PhysicsManager> SceneManager::GetPhysicsManager(std::string name)
{
    if(pManagers.find(name) != pManagers.end())
        return pManagers[name];
    Log::Write("Could not find a physics manager with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

std::shared_ptr<SoundManager> SceneManager::GetSoundManager()
{
    return sManager;
}

Camera* SceneManager::GetCamera()
{
    return camera;
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

void SceneManager::SetPhysicsManagerName(std::string name, std::string newName)
{
	auto it = pManagers.find(name);
	if(it != pManagers.end())
	{
		auto n = pManagers.extract(it);
		n.key() = newName;
		pManagers.insert(std::move(n));
	}
    else
        Log::Write("Could not find a physics manager with name \"" + name + "\"", Log::Type::Warning);
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

std::array<std::vector<std::string>, 5> SceneManager::GetNames()
{
	std::array<std::vector<std::string>, 5> ret;
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
    ret[3] = tmp; tmp.clear();
    for(auto& i : pManagers)
        tmp.push_back(i.first);
    ret[4] = tmp;
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
