#include <Engine.hpp>

int main()
{
    Json::Value cfg;
    Json::CharReaderBuilder rbuilder;

    std::ifstream file("launcher_conf.json");

    std::string errors;
    if(!Json::parseFromStream(rbuilder, file, &cfg, &errors))
        Log::Write("Launcher config parsing failed: " + errors, Log::Type::Critical);

    Engine engine(cfg["log"]["init"].asBool(), cfg["log"]["silent"].asBool());

    engine.CreateWindow(cfg["window"]["width"].asInt(), cfg["window"]["height"].asInt(), cfg["window"]["title"].asString(),
                        cfg["window"]["fullscreen"].asBool() ? sf::Style::Fullscreen : sf::Style::Default);
    engine.Init();

    engine.GetWindow().setVerticalSyncEnabled(cfg["window"]["vsync"].asBool());
    engine.GetWindow().setFramerateLimit(cfg["window"]["maxFps"].asInt());

    float guiWidth = cfg["gui"]["width"].asInt(), guiHeight = cfg["gui"]["height"].asInt();
    float bloomResolutionScale = cfg["renderer"]["bloomResolutionScale"].asFloat();

    engine.SetGuiView({ 0, 0, float(engine.GetWindow().getSize().x) * (guiWidth / float(engine.GetWindow().getSize().x)),
                              float(engine.GetWindow().getSize().y) * (guiHeight / float(engine.GetWindow().getSize().y)) });

    engine.SetGuiViewport({ 0, 0, float(engine.GetWindow().getSize().x), float(engine.GetWindow().getSize().y) });

    if(cfg["renderer"]["shadersDir"].asString() != "default")
        Renderer::GetInstance()->SetShadersDirectory(cfg["renderer"]["shadersDir"].asString());
    Renderer::GetInstance()->Init(engine.GetWindow().getSize(),
                                  cfg["renderer"]["hdriPath"].asString(),
                                  cfg["renderer"]["skyboxSideSize"].asInt(),
                                  cfg["renderer"]["irradianceSideSize"].asInt(),
                                  cfg["renderer"]["prefilteredSideSize"].asInt(),
                                  bloomResolutionScale);

    Camera cam(&engine.GetWindow(), { 0, 0, 0 });

    rp3d::PhysicsWorld::WorldSettings st;
    auto man = std::make_shared<PhysicsManager>(st);

    Light l({ 0, 0, 0 }, { 50.1, 100.0, 50.1 }, true);

    Material skyboxMaterial(
    {
        { Renderer::GetInstance()->GetTexture(Renderer::TextureType::Skybox), Material::Type::Cubemap }
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
        scene.AddLight(&l);

    bool manageCameraMovement = true, manageCameraLook = true,
         manageCameraMouse = true, manageSceneRendering = true,
         updateShadows = true, mouseCursorGrabbed = true,
         mouseCursorVisible = false;

    float exposure = cfg["renderer"]["exposure"].asFloat();
    float bloomStrength = 0.3;
    float mouseSensitivity = 1.0;

    float ssaoStrength = 2.0;
    float ssaoRadius = 0.5;

    int blurIterations = 8;

    ScriptManager scman;
    scman.AddProperty("Engine engine", &engine);
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
    scman.AddProperty("float mouseSensitivity", &mouseSensitivity);
    scman.AddProperty("float exposure", &exposure);
    scman.AddProperty("float bloomStrength", &bloomStrength);
    scman.AddProperty("int blurIterations", &blurIterations);
    scman.AddProperty("float ssaoStrength", &ssaoStrength);
    scman.AddProperty("float ssaoRadius", &ssaoRadius);
    scman.SetDefaultNamespace("");

    auto scPath = cfg["scenePath"].asString();
    scPath.insert(scPath.find_last_of('.'), "_scripts");
    scman.Load(scPath);

    scman.Build();
    
    if(!scman.IsBuildSucceded())
        return -1;

    scman.ExecuteFunction("void Start()");

    ShadowManager shadows(&scene, glm::ivec2(cfg["renderer"]["shadowMapResolution"].asInt()));

    engine.EventLoop([&](sf::Event& event)
    {
        if(event.type == sf::Event::Resized)
        {
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->Resize(event.size.width, event.size.height);
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Transparency)->Resize(event.size.width, event.size.height);
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong0)->Resize(event.size.width / bloomResolutionScale, event.size.height / bloomResolutionScale);
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong1)->Resize(event.size.width / bloomResolutionScale, event.size.height / bloomResolutionScale);

            engine.SetGuiView({ 0, 0, float(event.size.width) * (guiWidth / float(event.size.width)),
                                      float(event.size.height) * (guiHeight / float(event.size.height)) });

            engine.SetGuiViewport({ 0, 0, float(event.size.width), float(event.size.height) });
        }

        if(event.type == sf::Event::Closed) engine.Close();
    });

    engine.Loop([&]()
    {
        engine.GetWindow().setMouseCursorVisible(mouseCursorVisible);
        engine.GetWindow().setMouseCursorGrabbed(mouseCursorGrabbed);

        scman.ExecuteFunction("void Loop()");

        ListenerWrapper::SetPosition(cam.GetPosition());
        ListenerWrapper::SetOrientation(cam.GetOrientation());

        Renderer::GetInstance()->SetExposure(exposure);
        Renderer::GetInstance()->SetBloomStrength(bloomStrength);
        Renderer::GetInstance()->SetBlurIterations(blurIterations);
        Renderer::GetInstance()->SetSSAOStrength(ssaoStrength);
        Renderer::GetInstance()->SetSSAORadius(ssaoRadius);
        
        cam.Update();
        if(manageCameraMovement) cam.Move(1);
        if(manageCameraMouse) cam.Mouse(mouseSensitivity);
        if(manageCameraLook) cam.Look();
        
        if(updateShadows) shadows.Update();
        if(manageSceneRendering) scene.Draw(nullptr, nullptr, !updateShadows);

        Renderer::GetInstance()->DrawFramebuffers();
    });

    engine.Launch();
}
