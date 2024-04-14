#include <Engine.hpp>

Json::Value cfg;

struct Config
{
    Config()
    {
        multithreading = cfg["enableMultithreading"].asBool();

        width = cfg["window"]["width"].asInt();
        height = cfg["window"]["height"].asInt();
        fullscreen = cfg["window"]["fullscreen"].asBool();
        maxFps = cfg["window"]["maxFps"].asInt();
        vsync = cfg["window"]["vsync"].asBool();

        shadowMapResolution = cfg["renderer"]["shadowMapResolution"].asInt();
    }

    void Save()
    {
        cfg["enableMultithreading"] = multithreading;

        cfg["window"]["width"] = width;
        cfg["window"]["height"] = height;
        cfg["window"]["fullscreen"] = fullscreen;
        cfg["window"]["maxFps"] = maxFps;
        cfg["window"]["vsync"] = vsync;

        cfg["renderer"]["shadowMapResolution"] = shadowMapResolution;
        cfg["renderer"]["skyboxSideSize"] = Renderer::GetInstance().GetSkyboxResolution();
        cfg["renderer"]["irradianceSideSize"] = Renderer::GetInstance().GetIrradianceResolution();
        cfg["renderer"]["prefilteredSideSize"] = Renderer::GetInstance().GetPrefilteredResolution();
        cfg["renderer"]["ssaoSamples"] = Renderer::GetInstance().GetSSAOSamples();
        cfg["renderer"]["exposure"] = Renderer::GetInstance().GetExposure();
        cfg["renderer"]["ssaoStrength"] = Renderer::GetInstance().GetSSAOStrength();
        cfg["renderer"]["ssaoRadius"] = Renderer::GetInstance().GetSSAORadius();
        cfg["renderer"]["ssrEnabled"] = Renderer::GetInstance().IsSSREnabled();
        cfg["renderer"]["ssrRayStep"] = Renderer::GetInstance().GetSSRRayStep();
        cfg["renderer"]["ssrMaxSteps"] = Renderer::GetInstance().GetSSRMaxSteps();
        cfg["renderer"]["ssrMaxBinarySearchSteps"] = Renderer::GetInstance().GetSSRMaxBinarySearchSteps();
        cfg["renderer"]["ssgiEnabled"] = Renderer::GetInstance().IsSSGIEnabled();
        cfg["renderer"]["ssgiStrength"] = Renderer::GetInstance().GetSSGIStrength();

        std::ofstream file("launcher_conf.json");
        file << cfg.toStyledString();
        file.close();
    }

    uint32_t width, height, maxFps, shadowMapResolution;

    bool fullscreen, vsync, multithreading;
};

int main()
{
    Json::CharReaderBuilder rbuilder;

    std::ifstream file("launcher_conf.json");

    std::string errors;
    if(!Json::parseFromStream(rbuilder, file, &cfg, &errors))
        Log::Write("Launcher config parsing failed: " + errors, Log::Type::Critical);

    Multithreading::GetInstance().SetIsEnabled(cfg["enableMultithreading"].asBool());
    
    Engine engine(cfg["log"]["init"].asBool(), cfg["log"]["silent"].asBool());

    Config config;

    engine.CreateWindow(config.width, config.height, cfg["window"]["title"].asString(),
                        cfg["window"]["fullscreen"].asBool() ? sf::Style::Fullscreen : sf::Style::Default);
    engine.Init();

    engine.GetWindow().setVerticalSyncEnabled(config.vsync);
    engine.GetWindow().setFramerateLimit(config.maxFps);

    unsigned int guiWidth = cfg["gui"]["width"].asInt(), guiHeight = cfg["gui"]["height"].asInt();
    float bloomResolutionScale = cfg["renderer"]["bloomResolutionScale"].asFloat();
    if(bloomResolutionScale <= 0) bloomResolutionScale = 10.0;

    float renderResolutionScale = cfg["renderer"]["renderResolutionScale"].asFloat();
    if(renderResolutionScale <= 0) renderResolutionScale = 1.0;

    engine.SetGuiView({ 0, 0, float(engine.GetWindow().getSize().x) * (guiWidth / float(engine.GetWindow().getSize().x)),
                              float(engine.GetWindow().getSize().y) * (guiHeight / float(engine.GetWindow().getSize().y)) });

    engine.SetGuiViewport({ 0, 0, float(engine.GetWindow().getSize().x), float(engine.GetWindow().getSize().y) });

    if(cfg["renderer"]["shadersDir"].asString() != "default")
        Renderer::GetInstance().SetShadersDirectory(cfg["renderer"]["shadersDir"].asString());
    if(cfg["renderer"]["ssaoSamples"].asInt() > 0)
        Renderer::GetInstance().SetSSAOSamples(cfg["renderer"]["ssaoSamples"].asInt());
    Renderer::GetInstance().SetExposure(cfg["renderer"]["exposure"].asFloat());
    Renderer::GetInstance().SetSSAOStrength(cfg["renderer"]["ssaoStrength"].asFloat());
    Renderer::GetInstance().SetSSAORadius(cfg["renderer"]["ssaoRadius"].asFloat());
    Renderer::GetInstance().SetIsSSREnabled(cfg["renderer"]["ssrEnabled"].asBool());
    Renderer::GetInstance().SetSSRRayStep(cfg["renderer"]["ssrRayStep"].asFloat());
    Renderer::GetInstance().SetSSRMaxSteps(cfg["renderer"]["ssrMaxSteps"].asInt());
    Renderer::GetInstance().SetSSRMaxBinarySearchSteps(cfg["renderer"]["ssrMaxBinarySearchSteps"].asInt());

    Renderer::GetInstance().SetIsSSGIEnabled(cfg["renderer"]["ssgiEnabled"].asBool());
    Renderer::GetInstance().SetSSGIStrength(cfg["renderer"]["ssgiStrength"].asFloat());

    Renderer::GetInstance().Init(engine.GetWindow().getSize(),
                                  cfg["renderer"]["hdriPath"].asString(),
                                  cfg["renderer"]["skyboxSideSize"].asInt(),
                                  cfg["renderer"]["irradianceSideSize"].asInt(),
                                  cfg["renderer"]["prefilteredSideSize"].asInt(),
                                  bloomResolutionScale, cfg["renderer"]["useRGBA16F"].asBool());

    Camera cam(&engine.GetWindow(), { 0, 0, 0 });
    cam.SetGuiSize({ guiWidth, guiHeight });

    rp3d::PhysicsWorld::WorldSettings st;
    auto man = std::make_shared<PhysicsManager>(st);

    auto l = std::make_shared<Light>(rp3d::Vector3(0, 0, 0), rp3d::Vector3(50.1, 100.0, 50.1), true);

    Material skyboxMaterial(
    {
        { Renderer::GetInstance().GetTexture(Renderer::TextureType::Skybox), Material::Type::Cubemap }
    });

    auto skybox = std::make_shared<Model>(true);
    skybox->SetMaterial({ &skyboxMaterial });

    auto sman = std::make_shared<SoundManager>();

    SceneManager scene;
    scene.SetPhysicsManager(man);
    scene.SetCamera(&cam);
    scene.SetSkybox(skybox);
    scene.SetSoundManager(sman);
    scene.Load(cfg["scenePath"].asString());
    if(scene.GetNames()[2].empty())
        scene.AddLight(l);

    bool manageCameraMovement = true, manageCameraLook = true,
         manageCameraMouse = true, manageSceneRendering = true,
         updateShadows = true, mouseCursorGrabbed = true,
         mouseCursorVisible = false, windowLostFocus = false;

    float mouseSensitivity = 1.0;

    ScriptManager scman;
    scman.AddType("Config", sizeof(Config), { { "void Save()", WRAP_MFN(Config, Save) } },
    {
        { "uint width", asOFFSET(Config, width) },
        { "uint height", asOFFSET(Config, height) },
        { "uint maxFps", asOFFSET(Config, maxFps) },
        { "uint shadowMapResolution", asOFFSET(Config, shadowMapResolution) },
        { "bool fullscreen", asOFFSET(Config, fullscreen) },
        { "bool vsync", asOFFSET(Config, vsync) },
        { "bool multithreading", asOFFSET(Config, multithreading) }
    });
    scman.AddProperty("Engine engine", &engine);
    scman.AddProperty("Renderer renderer", &Renderer::GetInstance());
    scman.AddProperty("TextureManager textureManager", &TextureManager::GetInstance());
    scman.AddProperty("Config config", &config);
    scman.SetDefaultNamespace("Game");
    scman.AddProperty("SceneManager scene", &scene);
    scman.AddProperty("Camera camera", scene.GetCamera());
    scman.AddProperty("bool mouseCursorGrabbed", &mouseCursorGrabbed);
    scman.AddProperty("bool mouseCursorVisible", &mouseCursorVisible);
    scman.AddProperty("bool updateShadows", &updateShadows);
    scman.AddProperty("bool manageSceneRendering", &manageSceneRendering);
    scman.AddProperty("bool manageCameraMovement", &manageCameraMovement);
    scman.AddProperty("bool manageCameraLook", &manageCameraLook);
    scman.AddProperty("bool manageCameraMouse", &manageCameraMouse);
    scman.AddProperty("bool windowLostFocus", &windowLostFocus);
    scman.AddProperty("float mouseSensitivity", &mouseSensitivity);
    scman.SetDefaultNamespace("");

    auto scPath = cfg["scenePath"].asString();
    scPath.insert(scPath.find_last_of('.'), "_scripts");
    scman.Load(scPath, cfg["loadBytecode"].asBool());

    if(!cfg["loadBytecode"].asBool())
        scman.Build();
    
    if(!scman.IsBuildSucceded())
        return -1;

    scman.ExecuteFunction("void Start()");

    ShadowManager shadows(&scene, glm::ivec2(cfg["renderer"]["shadowMapResolution"].asInt()));

    engine.EventLoop([&](sf::Event& event)
    {
        if(event.type == sf::Event::Resized)
        {
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::GBuffer)->Resize(event.size.width / renderResolutionScale, event.size.height / renderResolutionScale);
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::DecalsGBuffer)->Resize(event.size.width / renderResolutionScale, event.size.height / renderResolutionScale);
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::Main)->Resize(event.size.width, event.size.height);
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::Transparency)->Resize(event.size.width, event.size.height);
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::SSAO)->Resize(event.size.width / 2.0, event.size.height / 2.0);
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::SSGI)->Resize(event.size.width / 2.0, event.size.height / 2.0);
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::SSR)->Resize(event.size.width, event.size.height);
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::Fog)->Resize(event.size.width, event.size.height);
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::BloomPingPong0)->Resize(event.size.width / bloomResolutionScale, event.size.height / bloomResolutionScale);
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::BloomPingPong1)->Resize(event.size.width / bloomResolutionScale, event.size.height / bloomResolutionScale);
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::SSGIPingPong0)->Resize(event.size.width / 8.0, event.size.height / 8.0);
            Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::SSGIPingPong1)->Resize(event.size.width / 8.0, event.size.height / 8.0);

            engine.SetGuiView({ 0, 0, float(event.size.width) * (guiWidth / float(event.size.width)),
                                      float(event.size.height) * (guiHeight / float(event.size.height)) });

            engine.SetGuiViewport({ 0, 0, float(event.size.width), float(event.size.height) });
        }

        if(event.type == sf::Event::LostFocus)
        {
            windowLostFocus = true;
            engine.GetWindow().setFramerateLimit(30);
        }
        else if(event.type == sf::Event::GainedFocus)
        {
            windowLostFocus = false;
            engine.GetWindow().setFramerateLimit(config.maxFps);
        }

        if(event.type == sf::Event::Closed) engine.Close();
    });

    engine.Loop([&]()
    {
        engine.GetWindow().setMouseCursorVisible(mouseCursorVisible || windowLostFocus);
        engine.GetWindow().setMouseCursorGrabbed(mouseCursorGrabbed || !windowLostFocus);

        scman.ExecuteFunction("void Loop()");

        ListenerWrapper::SetPosition(cam.GetPosition());
        ListenerWrapper::SetOrientation(cam.GetOrientation());
        
        if(!windowLostFocus)
        {
            cam.Update();
            if(manageCameraMovement) cam.Move(1);
            if(manageCameraMouse) cam.Mouse(mouseSensitivity);
            if(manageCameraLook) cam.Look();
        }
        
        if(manageSceneRendering && !windowLostFocus)
            scene.Draw(nullptr, nullptr, updateShadows ? shadows.Update() : true);
        else
        {
            scene.UpdateAnimations();
            scene.UpdatePhysics(true, true);
        }

        Renderer::GetInstance().DrawFramebuffers();
    });

    engine.Launch();
}
