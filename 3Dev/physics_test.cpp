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
    Camera cam(&engine.GetWindow(), &m, { 0, 10, 0 }, 0.5, 70, 0.001, 5000); // Main camera

    // Textures for a material
    GLuint texture = LoadTexture("../textures/metal_color.jpg"), normalmap = LoadTexture("../textures/metal_normal.jpg"),
    ao = LoadTexture("../textures/metal_ao.jpg"), metalness = LoadTexture("../textures/metal_metalness.jpg"),
    roughness = LoadTexture("../textures/metal_roughness.jpg");

    rp3d::PhysicsWorld::WorldSettings st; // Default physics world settings
    auto man = std::make_shared<PhysicsManager>(st); // Main physics manager

    Shader skyboxshader("../shaders/skybox.vs", "../shaders/skybox.frag"); // Shader for skybox rendering
    Shader shader("../shaders/vertex.vs", "../shaders/fragment.frag"); // Main shader
    Shader depth("../shaders/depth.vs", "../shaders/depth.frag"); // Depth shader
    Shader post("../shaders/post.vs", "../shaders/post.frag"); // Post-processing shader
    Shader environment("../shaders/environment.vs", "../shaders/environment.frag");
    Shader irradiance("../shaders/irradiance.vs", "../shaders/irradiance.frag");
    Shader spcfiltering("../shaders/spcfiltering.vs", "../shaders/spcfiltering.frag");
    Shader brdf("../shaders/brdf.vs", "../shaders/brdf.frag");

    Framebuffer buf(&post, 1280, 720), capture(nullptr, 256, 256),
                captureIrr(nullptr, 32, 32), captureSpc(nullptr, 256, 256),
                captureBRDF(&brdf, 512, 512);

    // Function for handling SFML window events
    engine.EventLoop([&](sf::Event& event)
    {
        if(event.type == sf::Event::Resized) // If the window is resized
            buf.RecreateTexture(event.size.width, event.size.height); // Resizing framebuffer texture

        if(event.type == sf::Event::Closed) engine.Close(); // Closing the window
    });

    // Main light
    Light l({ 3, 3, 3 }, { 0.1, 50.0, 0.1 });
    l.SetDirection({ 0.0, -1.0, 0.0 });
    /*l.SetCutoff(30);
    l.SetOuterCutoff(33);*/
    //l.SetAttenuation(0.0, 0.1, 0.0);

    Material envMat(
    {
        { LoadHDRTexture("../textures/outdoor.hdr"), Material::Type::Environment }
    });

    Shape captureCube({ 500, 500, 500 }, &envMat, &environment, &m, nullptr);
    GLuint cubemap = capture.CaptureCubemap(captureCube, m);
    envMat.GetParameters().clear();
    envMat.AddParameter(cubemap, Material::Type::Cubemap);
    captureCube.SetShader(&irradiance);
    GLuint irr = captureIrr.CaptureCubemap(captureCube, m, true);
    captureCube.SetShader(&spcfiltering);
    GLuint filtered = captureSpc.CaptureCubemapMipmaps(captureCube, m, 8, 1024);
    captureBRDF.Capture(0);
    GLuint BRDF = captureBRDF.GetTexture();
    cam.Update(true);

    Material material(
    {
    	{ texture, Material::Type::Color },
    	{ normalmap, Material::Type::Normal },
    	{ metalness, Material::Type::Metalness },
    	{ ao, Material::Type::AmbientOcclusion },
    	{ roughness, Material::Type::Roughness },
        { irr, Material::Type::Irradiance },
        { filtered, Material::Type::PrefilteredMap },
        { BRDF, Material::Type::LUT }
    });
    
    // All the shapes
    auto s = std::make_shared<Shape>(rp3d::Vector3{ 3, 3, 3 }, &material, &shader, &m, man.get());
    s->SetPosition({ 10, 30, 10 });

    auto s1 = std::make_shared<Shape>(rp3d::Vector3{ 3, 3, 3 }, &material, &shader, &m, man.get());
    s1->SetPosition({ 10, 36, 10 });

    auto s2 = std::make_shared<Shape>(rp3d::Vector3{ 1.5, 1.5, 3 }, &material, &shader, &m, man.get());
    s2->SetPosition({ 10, 13, 10 });
    
    auto skybox = std::make_shared<Shape>(rp3d::Vector3{ 500, 500, 500 }, &envMat, &skyboxshader, &m, nullptr);
    
    // Loading a sphere model
    auto sphere = std::make_shared<Model>("../sphere.obj", std::vector<Material>{ material }, &shader, &m, nullptr, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
    //sphere->CreateSphereShape(); // Creating sphere collision shape for a model
    sphere->SetPosition({ 10.f, 10.f, 10.f });

    auto terrain = std::make_shared<Model>("../terrain.obj", std::vector<Material>{ material }, &shader, &m, man.get(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
    terrain->CreateConcaveShape();
    terrain->SetPosition({ 10.f, -30.f, 10.f });

    SceneManager scene;
    scene.AddObject(s);
    scene.AddObject(s1);
    scene.AddObject(s2);

    scene.AddObject(sphere);
    scene.AddObject(terrain);

    scene.AddPhysicsManager(man);

    scene.AddLight(&l);
    
    scene.SetCamera(&cam);

    scene.SetSkybox(skybox);

    ShadowManager shadows(&scene, { &l }, &shader, &depth, glm::ivec2(2048, 2048));
    glEnable(GL_CULL_FACE);

    // Main game loop
    engine.Loop([&]() 
    {
   	    	
        // Camera movement, rotation and so on
        cam.Update();
        cam.Move(1);
        cam.Mouse();
        cam.Look();
        //////////////////////////////////////

        shadows.Update();
		
        scene.Draw(&buf);
        
        post.Bind();
        post.SetUniform1f("exposure", 1.0);

        buf.Draw();
    });

    // Launching the game!!!
    engine.Launch();
}
