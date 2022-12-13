#include <SceneManager.hpp>

void SceneManager::Draw(Framebuffer* fbo, Framebuffer* transparency, bool shadows)
{
    if(!fbo) fbo = Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main);

    fbo->Bind();
    auto size = fbo->GetSize();
    glViewport(0, 0, size.x, size.y);

    if(updatePhysics && !shadows)
    {
        float time = clock.restart().asSeconds();
        std::for_each(pManagers.begin(), pManagers.end(), [&](auto p) { p.second->Update(time); });
    }

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

    std::for_each(models.begin(), models.end(), [&](auto p) { p.second->Draw(camera, lightsVector, true); });
    std::for_each(shapes.begin(), shapes.end(), [&](auto p) { p.second->Draw(camera, lightsVector, true); });

    transparency->Unbind();
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
}

void SceneManager::AddObject(std::shared_ptr<Model> model, std::string name, bool checkUniqueness)
{
    auto n = ParseName(name);
    if(checkUniqueness)
    {
        int nameCount = std::count_if(models.begin(), models.end(), [&](auto& p)
                        { return p.first.find(n.first) != std::string::npos; });

        models[n.first + (nameCount ? std::to_string(nameCount) : "") + (n.second.empty() ? "" : ":") + n.second] = model;
    }
    else models[name] = model;

    if(!n.second.empty())
        modelGroups[n.second].push_back(model);
}

void SceneManager::AddObject(std::shared_ptr<Shape> shape, std::string name, bool checkUniqueness)
{
    auto n = ParseName(name);
    if(checkUniqueness)
    {
        int nameCount = std::count_if(shapes.begin(), shapes.end(), [&](auto& p)
                        { return p.first.find(n.first) != std::string::npos; });

        shapes[n.first + (nameCount ? std::to_string(nameCount) : "") + (n.second.empty() ? "" : ":") + n.second] = shape;
    }
    else shapes[name] = shape;

    if(!n.second.empty())
        shapeGroups[n.second].push_back(shape);
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
    if(it != models.end())
    {
        auto p = ParseName(it->first);
        models.erase(it);
        if(!p.second.empty())
            RemoveFromTheGroup(p.second, model);
    }
}

void SceneManager::RemoveObject(std::shared_ptr<Shape> shape)
{
    auto it = std::find_if(shapes.begin(), shapes.end(), [&](auto& p) { return p.second == shape; });
    if(it != shapes.end())
    {
        auto p = ParseName(it->first);
        shapes.erase(it);
        if(!p.second.empty())
            RemoveFromTheGroup(p.second, shape);
    }
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

    modelGroups.clear();
    shapeGroups.clear();
}

void SceneManager::Save(std::string filename, bool relativePaths)
{
    Json::Value data;
    int counter = 0;

    if(relativePaths)
        TextureManager::GetInstance()->MakeFilenamesRelativeTo(std::filesystem::path(filename).parent_path().string());

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
        std::string materialName = GetName(i.second->GetMaterial());
        data["objects"]["shapes"][counter]["material"] = materialName;
        counter++;
    }
    counter = 0;

    for(auto& i : models)
    {
        data["objects"]["models"][counter] = i.second->Serialize();
        if(relativePaths)
        {
            auto& modelFilename = data["objects"]["models"][counter]["filename"];
            modelFilename = std::filesystem::relative(modelFilename.asString(), std::filesystem::path(filename).parent_path()).string();
        }
        data["objects"]["models"][counter]["name"] = i.first;
        std::vector<std::string> materialNames;
        auto mat = i.second->GetMaterial();
        for(auto& i : mat)
            materialNames.push_back(GetName(i));
        for(int i = 0; i < materialNames.size(); i++)
            data["objects"]["models"][counter]["material"][i]["name"] = materialNames[i];
        counter++;
    }

    data["camera"] = camera->Serialize();

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

    std::filesystem::current_path(std::filesystem::path(filename).parent_path());

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
        auto shape = std::make_shared<Shape>(rp3d::Vector3::zero(), material, pManagers.begin()->second.get());
        shape->Deserialize(data["objects"]["shapes"][counter]);
        AddObject(shape, name, false);
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

        auto model = std::make_shared<Model>(data["objects"]["models"][counter]["filename"].asString(), material,
                                               aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes, pManagers.begin()->second.get());
        model->Deserialize(data["objects"]["models"][counter]);
        AddObject(model, name, false);
        counter++;
    }

    camera->Deserialize(data["camera"]);
}

void SceneManager::SaveState()
{
    savedState.clear();
    std::for_each(models.begin(), models.end(), [&](auto p)
                  {
                      savedState[p.first].pos = p.second->GetPosition();
                      savedState[p.first].size = p.second->GetSize();
                      savedState[p.first].orient = p.second->GetOrientation();
                      if(p.second->GetRigidBody())
                      {
                          p.second->GetRigidBody()->setLinearVelocity(rp3d::Vector3::zero());
                          p.second->GetRigidBody()->setAngularVelocity(rp3d::Vector3::zero());
                      }
                  });
    std::for_each(shapes.begin(), shapes.end(), [&](auto p)
                  {
                      savedState[p.first].pos = p.second->GetPosition();
                      savedState[p.first].size = p.second->GetSize();
                      savedState[p.first].orient = p.second->GetOrientation();
                      if(p.second->GetRigidBody())
                      {
                          p.second->GetRigidBody()->setLinearVelocity(rp3d::Vector3::zero());
                          p.second->GetRigidBody()->setAngularVelocity(rp3d::Vector3::zero());
                      }
                  });
}

void SceneManager::LoadState()
{
    for(auto& i : savedState)
    {
        if(models.find(i.first) != models.end())
        {
            models[i.first]->SetPosition(i.second.pos);
            models[i.first]->SetSize(i.second.size);
            models[i.first]->SetOrientation(i.second.orient);
        }
        else if(shapes.find(i.first) != shapes.end())
        {
            shapes[i.first]->SetPosition(i.second.pos);
            shapes[i.first]->SetSize(i.second.size);
            shapes[i.first]->SetOrientation(i.second.orient);
        }
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

void SceneManager::UpdatePhysics(bool update)
{
    updatePhysics = update;
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

std::vector<std::shared_ptr<Model>> SceneManager::GetModelGroup(std::string name)
{
    return modelGroups[name];
}

std::vector<std::shared_ptr<Shape>> SceneManager::GetShapeGroup(std::string name)
{
    return shapeGroups[name];
}

Model* SceneManager::GetModelPtr(std::string name)
{
    if(models.find(name) != models.end())
        return models[name].get();
    Log::Write("Could not find a model with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

Shape* SceneManager::GetShapePtr(std::string name)
{
    if(shapes.find(name) != shapes.end())
        return shapes[name].get();
    Log::Write("Could not find a shape with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

Material* SceneManager::GetMaterialPtr(std::string name)
{
    if(materials.find(name) != materials.end())
        return materials[name].get();
    Log::Write("Could not find a material with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

PhysicsManager* SceneManager::GetPhysicsManagerPtr(std::string name)
{
    if(pManagers.find(name) != pManagers.end())
        return pManagers[name].get();
    Log::Write("Could not find a physics manager with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

SoundManager* SceneManager::GetSoundManagerPtr()
{
    return sManager.get();
}

std::vector<Model*> SceneManager::GetModelPtrGroup(std::string name)
{
    std::vector<Model*> ret;
    for(auto& i : modelGroups[name])
        ret.push_back(i.get());
    return ret;
}

std::vector<Shape*> SceneManager::GetShapePtrGroup(std::string name)
{
    std::vector<Shape*> ret;
    for(auto& i : shapeGroups[name])
        ret.push_back(i.get());
    return ret;
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
        auto p = ParseName(name);
        auto p1 = ParseName(newName);

        if(p.second != p1.second)
            if(p1.second.empty())
                RemoveFromTheGroup(p.second, it->second);
            else if(p.second.empty())
                modelGroups[p1.second].push_back(it->second);
            else
                MoveToTheGroup(p.second, p1.second, it->second);

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
        auto p = ParseName(name);
        auto p1 = ParseName(newName);

        if(p.second != p1.second)
            if(p1.second.empty())
                RemoveFromTheGroup(p.second, it->second);
            else if(p.second.empty())
                shapeGroups[p1.second].push_back(it->second);
            else
                MoveToTheGroup(p.second, p1.second, it->second);

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

void SceneManager::RemoveFromTheGroup(std::string group, std::shared_ptr<Model> model)
{
    auto& groupVec = modelGroups[group];
    auto it = std::find(groupVec.begin(), groupVec.end(), model);
    groupVec.erase(it);
}

void SceneManager::RemoveFromTheGroup(std::string group, std::shared_ptr<Shape> shape)
{
    auto& groupVec = shapeGroups[group];
    auto it = std::find(groupVec.begin(), groupVec.end(), shape);
    groupVec.erase(it);
}

void SceneManager::MoveToTheGroup(std::string from, std::string to, std::shared_ptr<Model> model)
{
    auto& groupVec = modelGroups[from];
    auto it = std::find(groupVec.begin(), groupVec.end(), model);
    groupVec.erase(it);

    modelGroups[to].push_back(model);
}

void SceneManager::MoveToTheGroup(std::string from, std::string to, std::shared_ptr<Shape> shape)
{
    auto& groupVec = shapeGroups[from];
    auto it = std::find(groupVec.begin(), groupVec.end(), shape);
    groupVec.erase(it);

    shapeGroups[to].push_back(shape);
}

std::pair<std::string, std::string> SceneManager::ParseName(std::string in)
{
    if(in.find(':') == std::string::npos)
        return { in, "" };

    std::string name, group;
    name = in.substr(0, in.find(':'));
    group = in.substr(in.find(':') + 1, in.size());

    return { name, group };
}
