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

    Matrices m;

    Renderer renderer;
    renderer.Init(engine.GetWindow(), "../textures/park.hdr", m);

    Camera cam(&engine.GetWindow(), &m, { 0, 10, 0 }, 0.5, 70, 0.001, 5000); // Main camera

    // Textures for a material
    GLuint texture = LoadTexture("../textures/metal_color.jpg"), normalmap = LoadTexture("../textures/metal_normal.jpg"),
    ao = LoadTexture("../textures/metal_ao.jpg"), metalness = LoadTexture("../textures/metal_metalness.jpg"),
    roughness = LoadTexture("../textures/metal_roughness.jpg");

    rp3d::PhysicsWorld::WorldSettings st; // Default physics world settings
    auto man = std::make_shared<PhysicsManager>(st); // Main physics manager

    // Function for handling SFML window events
    engine.EventLoop([&](sf::Event& event)
    {
        if(event.type == sf::Event::Resized) // If the window is resized
            renderer.GetFramebuffer(Renderer::FramebufferType::Main)->RecreateTexture(event.size.width, event.size.height); // Resizing framebuffer texture

        if(event.type == sf::Event::Closed) engine.Close(); // Closing the window
    });

    // Main light
    Light l({ 3, 3, 3 }, { 0.1, 50.0, 0.1 });
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
    renderer.SetupMaterial(material);

    Material sphereMaterial(
    {
    	{ glm::vec3(0.8, 0.8, 0.8), Material::Type::Color },
    	{ glm::vec3(0.8), Material::Type::Metalness },
    	{ glm::vec3(0.4), Material::Type::Roughness }
    });
    renderer.SetupMaterial(sphereMaterial);

    Material skyboxMaterial(
    {
        { renderer.GetTexture(Renderer::TextureType::Skybox), Material::Type::Cubemap }
    });
    
    // All the shapes
    auto s = std::make_shared<Shape>(rp3d::Vector3{ 3, 3, 3 }, &material, renderer.GetShader(Renderer::ShaderType::Main), &m, man.get());
    s->SetPosition({ 10, 30, 10 });

    auto s1 = std::make_shared<Shape>(rp3d::Vector3{ 3, 3, 3 }, &material, renderer.GetShader(Renderer::ShaderType::Main), &m, man.get());
    s1->SetPosition({ 10, 36, 10 });

    auto s2 = std::make_shared<Shape>(rp3d::Vector3{ 1.5, 1.5, 3 }, &material, renderer.GetShader(Renderer::ShaderType::Main), &m, man.get());
    s2->SetPosition({ 10, 13, 10 });
    
    auto skybox = std::make_shared<Shape>(rp3d::Vector3{ 500, 500, 500 }, &skyboxMaterial, renderer.GetShader(Renderer::ShaderType::Skybox), &m, nullptr);
    
    // Loading a sphere model
    auto sphere = std::make_shared<Model>("../sphere.obj", std::vector<Material>{ sphereMaterial }, renderer.GetShader(Renderer::ShaderType::Main), &m, nullptr, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
    //sphere->CreateSphereShape(); // Creating sphere collision shape for a model
    sphere->SetPosition({ 10.f, 10.f, 10.f });

    auto terrain = std::make_shared<Model>("../terrain.obj", std::vector<Material>{ material }, renderer.GetShader(Renderer::ShaderType::Main), &m, man.get(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
    terrain->CreateConcaveShape();
    terrain->SetPosition({ 10.f, -30.f, 10.f });

    auto sman = std::make_shared<SoundManager>();
    sman->LoadSound("../quandale-dingle.ogg", "sound");

    SceneManager scene;

    scene.AddObject(s);
    scene.AddObject(s1);
    scene.AddObject(s2);

    scene.AddObject(sphere);
    scene.AddObject(terrain);

    scene.AddPhysicsManager(man);

    //scene.AddLight(&l);
    
    scene.SetCamera(&cam);

    scene.SetSkybox(skybox);

    scene.SetSoundManager(sman);

    scene.Save("hello.json");

    ShadowManager shadows(&scene, { &l }, renderer.GetShader(Renderer::ShaderType::Main), renderer.GetShader(Renderer::ShaderType::Depth), glm::ivec2(2048, 2048));

    ListenerWrapper::SetPosition(cam.GetPosition());
    ListenerWrapper::SetOrientation(cam.GetOrientation());

    sman->SetLoop(true, "sound");
    sman->SetAttenuation(3, "sound");
    sman->SetMinDistance(5, "sound");
    
    sman->PlayAt("sound", 0, sphere->GetPosition());

    // Main game loop
    engine.Loop([&]() 
    {
        // Camera movement, rotation and so on
        cam.Update();
        cam.Move(1);
        cam.Mouse();
        cam.Look();
        //////////////////////////////////////

        ListenerWrapper::SetPosition(cam.GetPosition());
        ListenerWrapper::SetOrientation(cam.GetOrientation());

        shadows.Update();
		
        scene.Draw(renderer.GetFramebuffer(Renderer::FramebufferType::Main));
        
        renderer.GetShader(Renderer::ShaderType::Post)->Bind();
        renderer.GetShader(Renderer::ShaderType::Main)->SetUniform1f("exposure", 1.5);

        //glDisable(GL_DEPTH_TEST);
        renderer.GetFramebuffer(Renderer::FramebufferType::Main)->Draw();
        //glEnable(GL_DEPTH_TEST);
        //transparency.Draw();
    });

    // Launching the game!!!
    engine.Launch();
    //scene.RemoveAllObjects();
    //engine.Launch();
}
