#include <Engine.hpp>

int main()
{
    Engine engine; // Engine class is responsible for creating a window

    engine.GetSettings().depthBits = 24; // Optional
    engine.GetSettings().antialiasingLevel = 4;
    engine.CreateWindow(1280, 720, "test"); // Creating a 1280x720 window with title "test"
    engine.Init(); // Initializing OpenGL

    engine.GetWindow().setMouseCursorVisible(false); // Hiding the cursor
    engine.GetWindow().setMouseCursorGrabbed(true); // Grabbing the cursor

    Renderer::GetInstance()->Init(engine.GetWindow().getSize(), "../textures/park.hdr");

    auto cam = std::make_shared<Camera>(&engine.GetWindow(), rp3d::Vector3{ 0, 10, 0 }); // Main camera

    // Textures for a material
    GLuint texture = TextureManager::GetInstance()->LoadTexture("../textures/metal_color.jpg"),
           normalmap = TextureManager::GetInstance()->LoadTexture("../textures/metal_normal.jpg"),
           ao = TextureManager::GetInstance()->LoadTexture("../textures/metal_ao.jpg"),
           metalness = TextureManager::GetInstance()->LoadTexture("../textures/metal_metalness.jpg"),
           roughness = TextureManager::GetInstance()->LoadTexture("../textures/metal_roughness.jpg");

    rp3d::PhysicsWorld::WorldSettings st; // Default physics world settings
    auto man = std::make_shared<PhysicsManager>(st); // Main physics manager

    // Function for handling SFML window events
    engine.EventLoop([&](sf::Event& event)
    {
        if(event.type == sf::Event::Resized) // If the window is resized
        {
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->RecreateTexture(event.size.width, event.size.height); // Resizing framebuffer texture
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Transparency)->RecreateTexture(event.size.width, event.size.height); // Resizing framebuffer texture
        }

        if(event.type == sf::Event::Closed) engine.Close(); // Closing the window
    });

    // Main light
    Light l({ 0, 0, 0 }, { 0.1, 50.0, 0.1 }, true);
    l.SetDirection({ 0.0, -1.0, 0.0 });
    /*l.SetCutoff(30);
    l.SetOuterCutoff(33);*/
    //l.SetAttenuation(0.0, 0.1, 0.0);

    Material material(
    {
    	{ texture, Material::Type::Color },
    	{ normalmap, Material::Type::Normal },
    	{ metalness, Material::Type::Metalness },
    	{ ao, Material::Type::AmbientOcclusion },
    	{ roughness, Material::Type::Roughness }
    });

    Material sphereMaterial(
    {
    	{ glm::vec3(0.8, 0.8, 0.8), Material::Type::Color },
    	{ glm::vec3(0.8), Material::Type::Metalness },
    	{ glm::vec3(0.4), Material::Type::Roughness }
    });

    Material skyboxMaterial(
    {
        { Renderer::GetInstance()->GetTexture(Renderer::TextureType::Skybox), Material::Type::Cubemap }
    });

    auto s = std::make_shared<Model>(true);
    s->SetPhysicsManager(man.get());
    s->CreateRigidBody();
    s->CreateBoxShape();
    s->SetMaterial({ &material });
    s->SetSize({ 3, 3, 3 });
    s->SetPosition({ 10, 30, 10 });

    auto s1 = std::make_shared<Model>(true);
    s1->SetPhysicsManager(man.get());
    s1->CreateRigidBody();
    s1->CreateBoxShape();
    s1->SetMaterial({ &material });
    s1->SetSize({ 3, 3, 3 });
    s1->SetPosition({ 10, 36, 10 });

    auto s2 = std::make_shared<Model>(true);
    s2->SetPhysicsManager(man.get());
    s2->CreateRigidBody();
    s2->CreateBoxShape();
    s2->SetMaterial({ &material });
    s2->SetSize({ 1.5, 1.5, 3 });
    s2->SetPosition({ 10, 13, 10 });

    auto skybox = std::make_shared<Model>(true);
    skybox->SetMaterial({ &skyboxMaterial });

    // Loading a sphere model
    auto sphere = std::make_shared<Model>("../sphere.obj", std::vector<Material*>{ &sphereMaterial }, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes, man.get());
    sphere->CreateSphereShape(); // Creating sphere collision shape for a model
    sphere->SetPosition({ 10.f, 10.f, 10.f });
    sphere->GetRigidBody()->setIsActive(false);

    auto terrain = std::make_shared<Model>("../terrain.obj", std::vector<Material*>{ &material }, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes, man.get());
    terrain->CreateConcaveShape();
    terrain->SetPosition({ 10.f, -30.f, 10.f });

    auto sman = std::make_shared<SoundManager>();
    sman->LoadSound("../quandale-dingle.ogg", "sound");

    SceneManager scene;

    scene.AddModel(s);
    scene.AddModel(s1);
    scene.AddModel(s2);

    scene.AddModel(sphere, "sphere");
    scene.AddModel(terrain);
    scene.SetPhysicsManager(man);

    scene.AddLight(&l);

    scene.SetCamera(cam.get());

    scene.SetSkybox(skybox);

    scene.SetSoundManager(sman);

    ShadowManager shadows(&scene, glm::ivec2(2048, 2048));

    ListenerWrapper::SetPosition(cam->GetPosition());
    ListenerWrapper::SetOrientation(cam->GetOrientation());

    sman->SetLoop(true, "sound");
    sman->SetAttenuation(3, "sound");
    sman->SetMinDistance(5, "sound");

    sman->PlayAt("sound", 0, sphere->GetPosition());

    bool manageCameraMovement = true, manageCameraLook = true,
         manageCameraMouse = true, manageSceneRendering = true,
         updateShadows = true, mouseCursorGrabbed = true,
         mouseCursorVisible = false;

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
    scman.LoadScript("../scripts/test.as");
    scman.SetDefaultNamespace("");
    scman.Build();

    scman.ExecuteFunction("void Start()");

    // Main game loop
    engine.Loop([&]()
    {
        engine.GetWindow().setMouseCursorVisible(mouseCursorVisible);
        engine.GetWindow().setMouseCursorGrabbed(mouseCursorGrabbed);

        // Camera movement, rotation and so on
        cam->Update();
        if(manageCameraMovement) cam->Move(1);
        if(manageCameraMouse) cam->Mouse();
        if(manageCameraLook) cam->Look();
        //////////////////////////////////////

        scman.ExecuteFunction("void Loop()");

        ListenerWrapper::SetPosition(cam->GetPosition());
        ListenerWrapper::SetOrientation(cam->GetOrientation());

        if(updateShadows) shadows.Update();
        if(manageSceneRendering) scene.Draw();

        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->Bind();
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1f("exposure", 1.0);

        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->Draw();
        glDisable(GL_DEPTH_TEST);
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Transparency)->Draw();
        glEnable(GL_DEPTH_TEST);
    });

    // Launching the game!!!
    engine.Launch();
}
