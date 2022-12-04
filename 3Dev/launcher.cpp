#include <Engine.hpp>

int main()
{
    Json::Value cfg;
    Json::CharReaderBuilder rbuilder;

    std::ifstream file("launcher_conf.json");

    std::string errors;
    if(!Json::parseFromStream(rbuilder, file, &cfg, &errors))
        Log::Write("Launcher config parsing failed: " + errors, Log::Type::Critical);

    Engine engine;

    engine.CreateWindow(cfg["window"]["width"].asInt(), cfg["window"]["height"].asInt(), cfg["window"]["title"].asString());
    engine.Init();

    engine.GetWindow().setMouseCursorVisible(false);
    engine.GetWindow().setMouseCursorGrabbed(true);

    if(cfg["renderer"]["shadersDir"].asString() != "default")
        Renderer::GetInstance()->SetShadersDirectory(cfg["renderer"]["shadersDir"].asString());
    Renderer::GetInstance()->Init(engine.GetWindow().getSize(),
                                  cfg["renderer"]["hdriPath"].asString(),
                                  cfg["renderer"]["skyboxSideSize"].asInt(),
                                  cfg["renderer"]["irradianceSideSize"].asInt(),
                                  cfg["renderer"]["prefilteredSideSize"].asInt());

    Camera cam(&engine.GetWindow(), { 0, 0, 0 });

    rp3d::PhysicsWorld::WorldSettings st;
    auto man = std::make_shared<PhysicsManager>(st);

    Light l({ 3, 3, 3 }, { 50.1, 150.0, 50.1 });
    l.SetDirection({ 0.0, -1.0, 0.0 });

    Material skyboxMaterial(
    {
        { Renderer::GetInstance()->GetTexture(Renderer::TextureType::Skybox), Material::Type::Cubemap }
    });

    auto skybox = std::make_shared<Shape>(rp3d::Vector3{ 1, 1, 1 }, &skyboxMaterial);

    SceneManager scene;
    scene.AddPhysicsManager(man);
    scene.SetCamera(&cam);
    scene.SetSkybox(skybox);
    scene.UpdatePhysics(false);
    scene.Load(cfg["scenePath"].asString());

    ScriptManager scman;
	bool manageCameraMovement = true, manageCameraLook = true, manageCameraMouse = true;
    scman.SetDefaultNamespace("Game");
    scman.AddProperty("SceneManager scene", &scene);
    scman.AddProperty("Camera camera", &cam);
    scman.AddProperty("bool manageCameraMovement", &manageCameraMovement);
    scman.AddProperty("bool manageCameraLook", &manageCameraLook);
    scman.AddProperty("bool manageCameraMouse", &manageCameraMouse);
    scman.SetDefaultNamespace("");

    auto scPath = cfg["scenePath"].asString();
    scPath.insert(scPath.find_last_of('.'), "_scripts");
    scman.Load(scPath);

    scman.Build();

    scman.ExecuteFunction("void Start()");

    scene.UpdatePhysics(true);

    ShadowManager shadows(&scene, { &l }, glm::ivec2(cfg["renderer"]["shadowMapResolution"].asInt()));

    float exposure = cfg["renderer"]["exposure"].asFloat();

    engine.EventLoop([&](sf::Event& event)
    {
        if(event.type == sf::Event::Resized)
        {
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->RecreateTexture(event.size.width, event.size.height);
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Transparency)->RecreateTexture(event.size.width, event.size.height);
        }

        if(event.type == sf::Event::Closed) engine.Close();
    });

    engine.Loop([&]()
    {
        cam.Update();
        if(manageCameraMovement) cam.Move(1);
        if(manageCameraMouse) cam.Mouse();
        if(manageCameraLook) cam.Look();

        scman.ExecuteFunction("void Loop()");

        shadows.Update();

        scene.Draw();

        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->Bind();
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1f("exposure", exposure);

        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->Draw();
        glDisable(GL_DEPTH_TEST);
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Transparency)->Draw();
        glEnable(GL_DEPTH_TEST);
    });

    engine.Launch();
}
