#include <SceneManager.hpp>

void SceneManager::Draw(Framebuffer* fbo, Framebuffer* transparency, bool updatePhysics, bool shadowPass)
{
    for(auto& i : animations)
    {
        auto actions = i.second->Update();
        for(auto& [name, transform] : actions)
        {
            if(nodes.find(name) != nodes.end())
            {
                nodes[name]->SetTransform(transform.first);
                nodes[name]->SetSize(transform.second);
            }
        }
    }

    if(!fbo) fbo = Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main);

    if(shadowPass)
    {
        if(this->updatePhysics && updatePhysics)
        {
            float time = clock.restart().asSeconds();
            pManager->Update(time);
        }
        else clock.restart();
        
        fbo->Bind();
        auto size = fbo->GetSize();
        glViewport(0, 0, size.x, size.y);
        std::for_each(models.begin(), models.end(), [&](auto p) 
            { if(!p.second->GetParent()) p.second->Draw(camera, lightsVector); });
        camera->Draw(camera, lightsVector);
        fbo->Unbind();
        return;
    }

    if(this->updatePhysics && updatePhysics)
    {
        float time = clock.restart().asSeconds();
        pManager->Update(time);
    }
    else clock.restart();

    SetMainShader(Renderer::GetInstance()->GetShader(Renderer::ShaderType::Deffered), true);

    auto gBuffer = Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::GBuffer);
    auto size = gBuffer->GetSize();
    glViewport(0, 0, size.x, size.y);
    gBuffer->Bind();

    for(int i = 0; i < 8; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

    std::for_each(models.begin(), models.end(), [&](auto p) 
        { if(!p.second->GetParent()) p.second->Draw(camera, lightsVector); });
    camera->Draw(camera, lightsVector);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer->GetTexture(false, 0));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer->GetTexture(false, 1));
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer->GetTexture(false, 2));
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gBuffer->GetTexture(false, 3));
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gBuffer->GetTexture(false, 4));

    auto lightingPass = Renderer::GetInstance()->GetShader(Renderer::ShaderType::LightingPass);
    lightingPass->Bind();

    lightingPass->SetUniform3f("campos", camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z);
    lightingPass->SetUniform1i("gposition", 0);
    lightingPass->SetUniform1i("galbedo", 1);
    lightingPass->SetUniform1i("gnormal", 2);
    lightingPass->SetUniform1i("gemission", 3);
    lightingPass->SetUniform1i("gcombined", 4);
    Material::UpdateShaderEnvironment(lightingPass);

    for(int i = 0; i < lightsVector.size(); i++)
        dynamic_cast<Light*>(lightsVector[i])->Update(lightingPass, i);

    size = fbo->GetSize();
    glViewport(0, 0, size.x, size.y);
    fbo->Bind();

    if(skybox)
    {
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        skybox->DrawSkybox();
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
    }

    gBuffer->Draw();

    fbo->Unbind();
    
    if(!transparency) transparency = Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Transparency);

    SetMainShader(Renderer::GetInstance()->GetShader(Renderer::ShaderType::Forward), true);

    glFrontFace(GL_CW);
    glDisable(GL_CULL_FACE);
    transparency->Bind();
    size = transparency->GetSize();
    glViewport(0, 0, size.x, size.y);

    for(int i = 0; i < 8; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

    std::for_each(models.begin(), models.end(), [&](auto p)
        { if(!p.second->GetParent()) p.second->Draw(camera, lightsVector, true); });
    camera->Draw(camera, lightsVector, true);

    transparency->Unbind();
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    auto post = Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post);

    post->Bind();
    glActiveTexture(GL_TEXTURE19);
	glBindTexture(GL_TEXTURE_2D, gBuffer->GetTexture(true));
    glActiveTexture(GL_TEXTURE20);
	glBindTexture(GL_TEXTURE_2D, transparency->GetTexture(true));

    post->SetUniform1i("frameDepth", 19);
    post->SetUniform1i("transparencyDepth", 20);
}

void SceneManager::AddModel(std::shared_ptr<Model> model, const std::string& name, bool checkUniqueness)
{
    auto n = ParseName(name);
    if(checkUniqueness)
    {
        int nameCount = std::count_if(models.begin(), models.end(), [&](auto& p)
                        { return p.first.find(n.first) != std::string::npos; });

        lastAdded = n.first + (nameCount ? std::to_string(nameCount) : "") + (n.second.empty() ? "" : ":") + n.second;
    }
    else lastAdded = name;

    models[lastAdded] = model;
    nodes[lastAdded] = (Node*)(model.get());

    if(!n.second.empty())
        modelGroups[n.second].push_back(model);

    for(auto& i : model->GetAnimations())
    {
        if(animations.find(i->GetName() + "-" + name) == animations.end())
        {
            std::vector<std::string> kfNames;
        
            for(auto& [name, kf] : i->GetKeyframes())
                kfNames.push_back(name);
            for(auto& j : kfNames)
            {
                auto n = i->GetKeyframes().extract(j);
                n.key() = j + "-" + lastAdded;
                i->GetKeyframes().insert(std::move(n));
            }
            animations[i->GetName() + "-" + lastAdded] = i;
        }
    }
}

void SceneManager::AddMaterial(std::shared_ptr<Material> material, const std::string& name)
{
    int nameCount = std::count_if(materials.begin(), materials.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });

    lastAdded = name + (nameCount ? std::to_string(nameCount) : "");
    materials[lastAdded] = material;
}

void SceneManager::AddAnimation(std::shared_ptr<Animation> animation, const std::string& name)
{
    int nameCount = std::count_if(animations.begin(), animations.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });

    lastAdded = name + (nameCount ? std::to_string(nameCount) : "");
    animations[lastAdded] = animation;
}

void SceneManager::AddLight(Light* light, const std::string& name)
{
    int nameCount = std::count_if(lights.begin(), lights.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });

    lastAdded = name + (nameCount ? std::to_string(nameCount) : "");
    nodes[lastAdded] = (Node*)(light);
    lights[lastAdded] = light;
    lightsVector.push_back(light);
}

void SceneManager::StoreBones(std::shared_ptr<Model> model, Bone* bone)
{
    if(!bone)
        for(auto& i : model->GetBones())
        {
            bones[i->GetName() + "-" + GetModelName(model)] = i.get();
            nodes[i->GetName() + "-" + GetModelName(model)] = i.get();
            StoreBones(model, i.get());
        }
    else
        for(auto i : bone->GetChildren())
        {
            auto b = dynamic_cast<Bone*>(i);
            bones[b->GetName() + "-" + GetModelName(model)] = b;
            nodes[b->GetName() + "-" + GetModelName(model)] = b;
            StoreBones(model, b);
        }
}

void SceneManager::RemoveBones(std::shared_ptr<Model> model, Bone* bone)
{
    if(!bone)
        for(auto& i : model->GetBones())
        {
            RemoveBones(model, i.get());
            bones.erase(i->GetName() + "-" + GetModelName(model));
            nodes.erase(i->GetName() + "-" + GetModelName(model));
        }
    else
        for(auto i : bone->GetChildren())
        {
            auto it = std::find_if(bones.begin(), bones.end(), [&](auto b) { return b.second == i; });
            if(it == bones.end())
            {
                i->SetParent(nullptr);
                continue;
            }
            else
            {
                auto b = dynamic_cast<Bone*>(i);
                RemoveBones(model, b);
                bones.erase(b->GetName() + "-" + GetModelName(model));
                nodes.erase(b->GetName() + "-" + GetModelName(model));
            }
        }
}

template<class... Args>
std::shared_ptr<Model> SceneManager::CreateModel(const std::string& name, Args&&... args)
{
    auto ret = std::make_shared<Model>(args...);
    AddModel(ret, name);
    return ret;
}

void SceneManager::RemoveModel(std::shared_ptr<Model> model)
{
    auto it = std::find_if(models.begin(), models.end(), [&](auto& p) { return p.second == model; });
    auto itn = std::find_if(nodes.begin(), nodes.end(), [&](auto& p) { return p.second == model.get(); });
    if(it != models.end())
    {
        auto p = ParseName(it->first);
        RemoveBones(model);
        models.erase(it);
        nodes.erase(itn);
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

void SceneManager::RemoveAnimation(std::shared_ptr<Animation> animation)
{
    auto it = std::find_if(animations.begin(), animations.end(), [&](auto& p) { return p.second == animation; });
    if(it != animations.end())
        animations.erase(it);
}

void SceneManager::RemoveLight(Light* light)
{
    auto it = std::find_if(lights.begin(), lights.end(), [&](auto& p) { return p.second == light; });
    auto itn = std::find_if(nodes.begin(), nodes.end(), [&](auto& p) { return p.second == light; });
    if(it != lights.end())
    {
        lights.erase(it);
        nodes.erase(itn);
        lightsVector.erase(std::find(lightsVector.begin(), lightsVector.end(), it->second));
    }
}

void SceneManager::RemoveAllObjects()
{
    models.clear();
    modelGroups.clear();
    lights.clear();
    nodes.clear();

    SetCamera(camera);
}

void SceneManager::Save(const std::string& filename, bool relativePaths)
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

    for(auto& i : bones)
    {
        data["bones"][counter] = i.second->Serialize();
        data["bones"][counter]["name"] = i.first;

        counter++;
    }
    counter = 0;

    for(auto& i : animations)
    {
        data["animations"][counter] = i.second->Serialize();
        data["animations"][counter]["name"] = i.first;

        counter++;
    }

    std::ofstream file(filename);
    file << data.toStyledString();
    file.close();
}

void SceneManager::Load(const std::string& filename, bool loadEverything)
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
    while(!data["animations"][counter].empty())
    {
        auto name = data["animations"][counter]["name"].asString();
        animations[name] = std::make_shared<Animation>(name);
        animations[name]->Deserialize(data["animations"][counter]);

        counter++;
    }
    counter = 0;

    std::vector<std::string> usedMaterials;
    while(!data["objects"]["models"][counter].empty())
    {
        for(auto& i : data["objects"]["models"][counter]["material"])
            usedMaterials.push_back(i["name"].asString());
        counter++;
    }
    counter = 0;

    while(!data["materials"][counter].empty())
    {
        auto name = data["materials"][counter]["name"].asString();
        materials[name] = std::make_shared<Material>();
        bool load = (std::find(usedMaterials.begin(), usedMaterials.end(), name) != usedMaterials.end() || loadEverything);
        materials[name]->Deserialize(data["materials"][counter], load);

        counter++;
    }
    counter = 0;
    
    while(!data["objects"]["models"][counter].empty())
    {
        auto name = data["objects"]["models"][counter]["name"].asString();
        auto filename = data["objects"]["models"][counter]["filename"].asString();

        std::vector<Material*> material;
        for(auto& i : data["objects"]["models"][counter]["material"])
            material.push_back(materials[i["name"].asString()].get());

        std::shared_ptr<Model> model;
        bool load = !filename.empty() && (data["objects"]["models"][counter]["immediateLoad"].asBool() || loadEverything);
        if(load)
            model = std::make_shared<Model>(filename, material, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes, pManager.get());
        else if(filename.empty())
        {
            model = std::make_shared<Model>(true);
            model->SetMaterial(material);
            model->SetPhysicsManager(pManager.get());
            model->CreateRigidBody();
        }
        else
        {
            model = std::make_shared<Model>();
            model->SetFilename(filename);
            model->SetMaterial(material);
            model->SetPhysicsManager(pManager.get());
            model->CreateRigidBody();
        }
        model->Deserialize(data["objects"]["models"][counter], (!load && !filename.empty()));
        AddModel(model, name, false);
        StoreBones(model);

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

    while(!data["bones"][counter].empty())
    {
        if(bones.find(data["bones"][counter]["name"].asString()) != bones.end())
            bones[data["bones"][counter]["name"].asString()]->Deserialize(data["bones"][counter]);

        counter++;
    }
    counter = 0;

    camera->Deserialize(data["camera"]);

    while(!data["objects"]["models"][counter].empty())
    {
        auto model = GetModel(data["objects"]["models"][counter]["name"].asString());
        for(auto& i : data["objects"]["models"][counter]["children"])
            if(bones.find(i.asString()) == bones.end())
                model->AddChild(GetNode(i.asString()));
        auto parent = data["objects"]["models"][counter]["parent"].asString();
        if(!data["objects"]["models"][counter]["parent"].empty())
        {
            model->SetParent(GetNode(parent));
            if(bones.find(parent) != bones.end())
                GetNode(parent)->AddChild(model.get());
        }

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
    std::for_each(nodes.begin(), nodes.end(), [&](auto p)
                  {
                      savedState[p.first].pos = p.second->GetTransform().getPosition();
                      savedState[p.first].size = p.second->GetSize();
                      savedState[p.first].orient = p.second->GetTransform().getOrientation();
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
        if(nodes.find(i.first) != nodes.end())
        {
            nodes[i.first]->SetTransform(rp3d::Transform(i.second.pos, i.second.orient));
            nodes[i.first]->SetSize(i.second.size);
        }
    }
    
    for(auto& i : temporaryModelCopies)
        RemoveModel(GetModel(i));

    temporaryModelCopies.clear();
}

void SceneManager::LoadEnvironment(const std::string& filename)
{
    Renderer::GetInstance()->LoadEnvironment(filename);
    skybox->GetMaterial()[0]->SetParameter(Renderer::GetInstance()->GetTexture(Renderer::TextureType::Skybox), Material::Type::Cubemap);
}

void SceneManager::SetMainShader(Shader* shader, bool temp)
{
    std::for_each(models.begin(), models.end(), [&](auto p) { p.second->SetShader(shader, temp); });
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

std::shared_ptr<Model> SceneManager::GetModel(const std::string& name)
{
    if(models.find(name) != models.end())
        return models[name];
    Log::Write("Could not find a model with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

std::shared_ptr<Material> SceneManager::GetMaterial(const std::string& name)
{
    if(materials.find(name) != materials.end())
        return materials[name];
    Log::Write("Could not find a material with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

std::shared_ptr<Animation> SceneManager::GetAnimation(const std::string& name)
{
    if(animations.find(name) != animations.end())
        return animations[name];
    Log::Write("Could not find an animation with name \""
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

Node* SceneManager::GetNode(const std::string& name)
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
    auto it = std::find_if(nodes.begin(), nodes.end(), [&](auto n)
                {
                    return n.second == node;
                });
    return it != nodes.end() ? it->first : "";
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

std::vector<std::shared_ptr<Model>> SceneManager::GetModelGroup(const std::string& name)
{
    return modelGroups[name];
}

Model* SceneManager::GetModelPtr(const std::string& name)
{
    if(models.find(name) != models.end())
        return models[name].get();
    Log::Write("Could not find a model with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

Material* SceneManager::GetMaterialPtr(const std::string& name)
{
    if(materials.find(name) != materials.end())
        return materials[name].get();
    Log::Write("Could not find a material with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

Animation* SceneManager::GetAnimationPtr(const std::string& name)
{
    if(animations.find(name) != animations.end())
        return animations[name].get();
    Log::Write("Could not find an animation with name \""
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

void SceneManager::RemoveModelPtr(Model* model)
{
    auto it = std::find_if(models.begin(), models.end(), [&](auto& p) { return p.second.get() == model; });
    auto itn = std::find_if(nodes.begin(), nodes.end(), [&](auto& p) { return p.second == model; });
    if(it != models.end())
    {
        auto p = ParseName(it->first);
        RemoveBones(it->second);
        if(!p.second.empty())
            RemoveFromTheGroup(p.second, it->second);
        models.erase(it);
        nodes.erase(itn);
    }
}

void SceneManager::RemoveMaterialPtr(Material* material)
{
    auto it = std::find_if(materials.begin(), materials.end(), [&](auto& p) { return p.second.get() == material; });
    if(it != materials.end())
        materials.erase(it);
}

void SceneManager::RemoveAnimationPtr(Animation* animation)
{
    auto it = std::find_if(animations.begin(), animations.end(), [&](auto& p) { return p.second.get() == animation; });
    if(it != animations.end())
        animations.erase(it);
}

Model* SceneManager::CloneModel(Model* model, bool isTemporary, const std::string& name)
{
    auto ret = std::make_shared<Model>(model);
    AddModel(ret, name);
    if(model->GetAnimations().empty())
    {
        for(auto& i : animations)
        {
            if(i.first.find("-" + GetModelName(model)) != std::string::npos)
            {
                auto animName = i.second->GetName().empty() ? i.first.substr(0, i.first.find_last_of("-")) : i.second->GetName();
                animations[animName + "-" + lastAdded] = std::make_shared<Animation>(*i.second.get());
                std::vector<std::string> kfNames;
    
                for(auto& [name, kf] : animations[animName + "-" + lastAdded]->GetKeyframes())
                    kfNames.push_back(name);
                for(auto& j : kfNames)
                {
                    if(bones.find(j) != bones.end())
                    {
                        auto n = animations[animName + "-" + lastAdded]->GetKeyframes().extract(j);
                        n.key() = j.substr(0, j.find_last_of("-")) + "-" + lastAdded;
                        animations[animName + "-" + lastAdded]->GetKeyframes().insert(std::move(n));
                    }
                }
            }
        }
    }
    StoreBones(ret);
    if(isTemporary)
        temporaryModelCopies.push_back(GetLastAdded());
    return ret.get();
}

std::vector<Model*> SceneManager::GetModelPtrGroup(const std::string& name)
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

Light* SceneManager::GetLight(const std::string& name)
{
    if(lights.find(name) != lights.end())
        return lights[name];
    Log::Write("Could not find a light with name \""
                + name + "\", function will return nullptr", Log::Type::Warning);
    return nullptr;
}

Bone* SceneManager::GetBone(const std::string& name)
{
    if(bones.find(name) != bones.end())
        return bones[name];
    Log::Write("Could not find a bone with name \""
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

void SceneManager::SetModelName(const std::string& name, const std::string& newName)
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

        std::vector<std::string> modelBones;
        std::vector<std::string> modelAnims;

        for(auto& [boneName, bone] : bones)
            if(boneName.find(name) != std::string::npos)
                modelBones.push_back(boneName);

        for(auto& [animName, anim] : animations)
            if(animName.find(name) != std::string::npos)
                modelAnims.push_back(animName);

        for(auto& i : modelBones)
        {
            auto n = bones.extract(i);
            n.key() = i.substr(0, i.find(name)) + newName;
            bones.insert(std::move(n));

            auto n1 = nodes.extract(i);
            n1.key() = i.substr(0, i.find(name)) + newName;
            nodes.insert(std::move(n1));
        }

        for(auto& i : modelAnims)
        {
            auto n = animations.extract(i);
            n.key() = i.substr(0, i.find(name)) + newName;
            animations.insert(std::move(n));
        }
	}
    else
        Log::Write("Could not find a model with name \"" + name + "\"", Log::Type::Warning);
}

void SceneManager::SetMaterialName(const std::string& name, const std::string& newName)
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

void SceneManager::SetAnimationName(const std::string& name, const std::string& newName)
{
    auto it = animations.find(name);
	if(it != animations.end())
	{
		auto n = animations.extract(it);
		n.key() = newName;
		animations.insert(std::move(n));
	}
    else
        Log::Write("Could not find an animation with name \"" + name + "\"", Log::Type::Warning);
}

void SceneManager::SetLightName(const std::string& name, const std::string& newName)
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

std::array<std::vector<std::string>, 6> SceneManager::GetNames()
{
	std::array<std::vector<std::string>, 6> ret;
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
    for(auto& i : bones)
        tmp.push_back(i.first);
    ret[4] = tmp; tmp.clear();
    for(auto& i : animations)
        tmp.push_back(i.first);
    ret[5] = tmp;
    return ret;
}

void SceneManager::RemoveFromTheGroup(const std::string& group, std::shared_ptr<Model> model)
{
    auto& groupVec = modelGroups[group];
    auto it = std::find(groupVec.begin(), groupVec.end(), model);
    groupVec.erase(it);
}

void SceneManager::MoveToTheGroup(const std::string& from, const std::string& to, std::shared_ptr<Model> model)
{
    auto& groupVec = modelGroups[from];
    auto it = std::find(groupVec.begin(), groupVec.end(), model);
    groupVec.erase(it);

    modelGroups[to].push_back(model);
}

std::pair<std::string, std::string> SceneManager::ParseName(const std::string& in)
{
    if(in.find(':') == std::string::npos)
        return { in, "" };

    std::string name, group;
    name = in.substr(0, in.find(':'));
    group = in.substr(in.find(':') + 1, in.size());

    return { name, group };
}
