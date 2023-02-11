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
        pManager->Update(time);
    }
    else clock.restart();

    // needed for materials without textures to render correctly in some cases
    for(int i = 0; i < 8; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

    std::for_each(models.begin(), models.end(), [&](auto p) { p.second->Draw(camera, lightsVector); });

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

    transparency->Unbind();
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
}

void SceneManager::AddModel(std::shared_ptr<Model> model, std::string name, bool checkUniqueness)
{
    auto n = ParseName(name);
    if(checkUniqueness)
    {
        int nameCount = std::count_if(models.begin(), models.end(), [&](auto& p)
                        { return p.first.find(n.first) != std::string::npos; });

        lastAdded = n.first + (nameCount ? std::to_string(nameCount) : "") + (n.second.empty() ? "" : ":") + n.second;
        models[lastAdded] = model;
        nodes[lastAdded] = (Node*)(model.get());
    }
    else
    {
        models[name] = model;
        nodes[name] = (Node*)(model.get());
        lastAdded = name;
    }

    if(!n.second.empty())
        modelGroups[n.second].push_back(model);
}

void SceneManager::AddMaterial(std::shared_ptr<Material> material, std::string name)
{
    int nameCount = std::count_if(materials.begin(), materials.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });

    lastAdded = name + (nameCount ? std::to_string(nameCount) : "");
    materials[lastAdded] = material;
}

void SceneManager::AddLight(Light* light, std::string name)
{
    int nameCount = std::count_if(lights.begin(), lights.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });

    lastAdded = name + (nameCount ? std::to_string(nameCount) : "");
    nodes[lastAdded] = (Node*)(light);
    lights[lastAdded] = light;
    lightsVector.push_back(light);
}

template<class... Args>
std::shared_ptr<Model> SceneManager::CreateModel(std::string name, Args&&... args)
{
    auto ret = std::make_shared<Model>(args...);
    AddModel(ret, name);
    return ret;
}

void SceneManager::RemoveModel(std::shared_ptr<Model> model)
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

void SceneManager::RemoveMaterial(std::shared_ptr<Material> material)
{
    auto it = std::find_if(materials.begin(), materials.end(), [&](auto& p) { return p.second == material; });
    if(it != materials.end())
        materials.erase(it);
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
    modelGroups.clear();
    lights.clear();
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

    for(auto& i : models)
    {
        data["objects"]["models"][counter] = i.second->Serialize();
        if(relativePaths)
        {
            auto& modelFilename = data["objects"]["models"][counter]["filename"];
            if(!modelFilename.empty())
                modelFilename = std::filesystem::relative(modelFilename.asString(), std::filesystem::path(filename).parent_path()).string();
        }
        data["objects"]["models"][counter]["name"] = i.first;
        std::vector<std::string> materialNames;
        auto mat = i.second->GetMaterial();
        for(auto& j : mat)
            materialNames.push_back(GetMaterialName(j));
        for(int j = 0; j < materialNames.size(); j++)
            data["objects"]["models"][counter]["material"][j]["name"] = materialNames[j];
        for(int j = 0; j < i.second->GetChildren().size(); j++)
            data["objects"]["models"][counter]["children"][j] = GetNodeName(i.second->GetChildren()[j]);
        if(i.second->GetParent())
            data["objects"]["models"][counter]["parent"] = GetNodeName(i.second->GetParent());

        counter++;
    }
    counter = 0;

    for(auto& i : lights)
    {
        data["lights"][counter] = i.second->Serialize();
        data["lights"][counter]["name"] = i.first;

        for(int j = 0; j < i.second->GetChildren().size(); j++)
            data["lights"][counter]["children"][j] = GetNodeName(i.second->GetChildren()[j]);
        if(i.second->GetParent())
            data["lights"][counter]["parent"] = GetNodeName(i.second->GetParent());

        counter++;
    }
    counter = 0;

    data["camera"] = camera->Serialize();

    for(int j = 0; j < camera->GetChildren().size(); j++)
        data["camera"]["children"][j] = GetNodeName(camera->GetChildren()[j]);
    if(camera->GetParent())
        data["camera"]["parent"] = GetNodeName(camera->GetParent());

    data["sounds"] = sManager->Serialize(relativePaths ? filename : "");

    std::ofstream file(filename);
    file << data.toStyledString();
    file.close();
}

void SceneManager::Load(std::string filename)
{
    RemoveAllObjects();

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
    // For old projects with shapes
    while(!data["objects"]["shapes"][counter].empty())
    {
        auto name = data["objects"]["shapes"][counter]["name"].asString();
        auto material = materials[data["objects"]["shapes"][counter]["material"].asString()].get();
        auto shape = std::make_shared<Model>(true);
        shape->SetMaterial({ material });
        shape->SetPhysicsManager(pManager.get());
        shape->CreateRigidBody();
        shape->CreateBoxShape();
        shape->Deserialize(data["objects"]["shapes"][counter]);
        AddModel(shape, name, false);
        counter++;
    }
    counter = 0;
    
    while(!data["objects"]["models"][counter].empty())
    {
        auto name = data["objects"]["models"][counter]["name"].asString();
        auto filename = data["objects"]["models"][counter]["filename"].asString();

        //auto material = materials[data["objects"]["models"][counter]["material"].asString()].get();
        std::vector<Material*> material;
        for(auto& i : data["objects"]["models"][counter]["material"])
            material.push_back(materials[i["name"].asString()].get());

        std::shared_ptr<Model> model;
        if(!filename.empty())
            model = std::make_shared<Model>(filename, material, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes, pManager.get());
        else
        {
            model = std::make_shared<Model>(true);
            model->SetMaterial(material);
            model->SetPhysicsManager(pManager.get());
            model->CreateRigidBody();
        }
        model->Deserialize(data["objects"]["models"][counter]);
        AddModel(model, name, false);
        counter++;
    }
    counter = 0;

    while(!data["lights"][counter].empty())
    {
        auto name = data["lights"][counter]["name"].asString();
        lights[name] = new Light(rp3d::Vector3::zero(), rp3d::Vector3::zero());
        lights[name]->Deserialize(data["lights"][counter]);
        nodes[name] = lights[name];
        lightsVector.push_back(lights[name]);
        counter++;
    }
    counter = 0;

    camera->Deserialize(data["camera"]);

    while(!data["objects"]["models"][counter].empty())
    {
        auto model = GetModel(data["objects"]["models"][counter]["name"].asString());
        for(auto& i : data["objects"]["models"][counter]["children"])
            model->AddChild(GetNode(i.asString()));
        if(!data["objects"]["models"][counter]["parent"].empty())
            model->SetParent(GetNode(data["objects"]["models"][counter]["parent"].asString()));
        counter++;
    }
    counter = 0;

    while(!data["lights"][counter].empty())
    {
        auto light = GetLight(data["lights"][counter]["name"].asString());
        for(auto& i : data["lights"][counter]["children"])
            light->AddChild(GetNode(i.asString()));
        if(!data["lights"][counter]["parent"].empty())
            light->SetParent(GetNode(data["lights"][counter]["parent"].asString()));
        counter++;
    }
    counter = 0;

    for(auto& i : data["camera"]["children"])
        camera->AddChild(GetNode(i.asString()));
    if(!data["camera"]["parent"].empty())
        camera->SetParent(GetNode(data["camera"]["parent"].asString()));

    sManager->Deserialize(data["sounds"]);
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
    }
    
    for(auto& i : temporaryModelCopies)
        RemoveModel(GetModel(i));

    temporaryModelCopies.clear();
}

void SceneManager::SetMainShader(Shader* shader)
{
    std::for_each(models.begin(), models.end(), [&](auto p) { p.second->SetShader(shader); });
}

void SceneManager::SetCamera(Camera* camera)
{
    this->camera = camera;
    nodes["camera"] = (Node*)(camera);
}

void SceneManager::SetSkybox(std::shared_ptr<Model> skybox)
{
    this->skybox = skybox;
}

void SceneManager::SetPhysicsManager(std::shared_ptr<PhysicsManager> manager)
{
    pManager = manager;
}

void SceneManager::SetSoundManager(std::shared_ptr<SoundManager> manager)
{
    sManager = manager;
}

void SceneManager::UpdatePhysics(bool update)
{
    updatePhysics = update;
}

std::string SceneManager::GetLastAdded()
{
    return lastAdded;
}

std::shared_ptr<Model> SceneManager::GetModel(std::string name)
{
    if(models.find(name) != models.end())
        return models[name];
    Log::Write("Could not find a model with name \""
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

std::shared_ptr<PhysicsManager> SceneManager::GetPhysicsManager()
{
    return pManager;
}

std::shared_ptr<SoundManager> SceneManager::GetSoundManager()
{
    return sManager;
}

Node* SceneManager::GetNode(std::string name)
{
    if(nodes.find(name) != nodes.end())
        return nodes[name];
    Log::Write("Could not find a node with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

std::string SceneManager::GetModelName(std::shared_ptr<Model> model)
{
    return std::find_if(models.begin(), models.end(), [&](auto m)
            {
                return m.second == model;
            })->first;
}

std::string SceneManager::GetModelName(Model* model)
{
    return std::find_if(models.begin(), models.end(), [&](auto m)
            {
                return m.second.get() == model;
            })->first;
}

std::string SceneManager::GetNodeName(Node* node)
{
    return std::find_if(nodes.begin(), nodes.end(), [&](auto n)
            {
                return n.second == node;
            })->first;
}

std::string SceneManager::GetMaterialName(std::shared_ptr<Material> mat)
{
    return std::find_if(materials.begin(), materials.end(), [&](auto m)
            {
                return m.second == mat;
            })->first;
}

std::string SceneManager::GetMaterialName(Material* mat)
{
    return std::find_if(materials.begin(), materials.end(), [&](auto m)
            {
                return m.second.get() == mat;
            })->first;
}

std::vector<std::shared_ptr<Model>> SceneManager::GetModelGroup(std::string name)
{
    return modelGroups[name];
}

Model* SceneManager::GetModelPtr(std::string name)
{
    if(models.find(name) != models.end())
        return models[name].get();
    Log::Write("Could not find a model with name \""
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

PhysicsManager* SceneManager::GetPhysicsManagerPtr()
{
    return pManager.get();
}

SoundManager* SceneManager::GetSoundManagerPtr()
{
    return sManager.get();
}

Model* SceneManager::CloneModel(Model* model, bool isTemporary, std::string name)
{
    auto ret = std::make_shared<Model>(model);
    AddModel(ret, name);
    if(isTemporary)
        temporaryModelCopies.push_back(GetLastAdded());
    return ret.get();
}

std::vector<Model*> SceneManager::GetModelPtrGroup(std::string name)
{
    std::vector<Model*> ret;
    for(auto& i : modelGroups[name])
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

std::vector<Light*> SceneManager::GetShadowCastingLights()
{
    std::vector<Light*> ret;
    for(auto& i : lights)
        if(i.second->IsCastingShadows())
            ret.push_back(i.second);
    return ret;
}

void SceneManager::SetModelName(std::string name, std::string newName)
{
	auto it = models.find(name);
    auto itn = nodes.find(name);
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

        auto n1 = nodes.extract(itn);
		n1.key() = newName;
		nodes.insert(std::move(n1));
	}
    else
        Log::Write("Could not find a model with name \"" + name + "\"", Log::Type::Warning);
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
    auto itn = nodes.find(name);
	if(it != lights.end())
	{
		auto n = lights.extract(it);
		n.key() = newName;
		lights.insert(std::move(n));

        auto n1 = nodes.extract(itn);
		n1.key() = newName;
		nodes.insert(std::move(n1));
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
    for(auto& i : materials)
        tmp.push_back(i.first);
    ret[1] = tmp; tmp.clear();
    for(auto& i : lights)
        tmp.push_back(i.first);
    ret[2] = tmp; tmp.clear();
    for(auto& i : nodes)
        tmp.push_back(i.first);
    ret[3] = tmp; tmp.clear();
    return ret;
}

void SceneManager::RemoveFromTheGroup(std::string group, std::shared_ptr<Model> model)
{
    auto& groupVec = modelGroups[group];
    auto it = std::find(groupVec.begin(), groupVec.end(), model);
    groupVec.erase(it);
}

void SceneManager::MoveToTheGroup(std::string from, std::string to, std::shared_ptr<Model> model)
{
    auto& groupVec = modelGroups[from];
    auto it = std::find(groupVec.begin(), groupVec.end(), model);
    groupVec.erase(it);

    modelGroups[to].push_back(model);
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
