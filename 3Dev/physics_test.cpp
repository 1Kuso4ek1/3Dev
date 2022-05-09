#include <Engine.hpp>

/*GLuint GenerateIrradiance(GLuint texture)
{
    Shader shader("../shaders/irradiance.vs", "../shaders/irradiance.frag");
    Framebuffer buf(&shader, 1024, 512, false, GL_LINEAR);
    glViewport(0, 0, 1024, 512);
    buf.Capture(texture);
    return buf.GetTexture();
}*/

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
    
    // Filenames for a cubemap
    /*std::vector<std::string> skybox_textures = 
    {
        "../textures/skybox_right.bmp",
        "../textures/skybox_left.bmp",
        "../textures/skybox_top.bmp",
        "../textures/skybox_bottom.bmp",
        "../textures/skybox_front.bmp",
        "../textures/skybox_back.bmp"
    };
    GLuint cubemap = LoadCubemap(skybox_textures);*/
    /*GLuint hdrmap = LoadHDRTexture("ref.hdr");
    GLuint irradiance = LoadHDRTexture("env.hdr");*/

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

    Framebuffer buf(&post, 1280, 720); // Main framebuffer

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

    // Main material
    Material material(
    { 
    	{ glm::vec3(1.0, 0.0, 0.0), Material::Type::Color },
    	//{ normalmap, Material::Type::Normal },
    	{ glm::vec3(0.2), Material::Type::Metalness },
    	//{ ao, Material::Type::AmbientOcclusion },
    	{ glm::vec3(0.7), Material::Type::Roughness }
        //{ irradiance, Material::Type::Irradiance }
    });

    // Material for the skybox
    /*Material skybox_mat(
    {
        { cubemap, Material::Type::Cubemap }
    });*/

    Material env_mat(
    {
        { LoadTexture("../textures/outdoor.hdr"), Material::Type::Environment }
    });
    
    // All the shapes
    auto env = std::make_shared<Shape>(rp3d::Vector3{ 1000, 1000, 1000 }, &env_mat, &environment, &m, nullptr);
    env->SetPosition({ 10, 40, 10 });

    auto s = std::make_shared<Shape>(rp3d::Vector3{ 3, 3, 3 }, &material, &shader, &m, man.get());
    s->SetPosition({ 10, 30, 10 });

    auto s1 = std::make_shared<Shape>(rp3d::Vector3{ 3, 3, 3 }, &material, &shader, &m, man.get());
    s1->SetPosition({ 10, 36, 10 });

    auto s2 = std::make_shared<Shape>(rp3d::Vector3{ 1.5, 1.5, 3 }, &material, &shader, &m, man.get());
    s2->SetPosition({ 10, 13, 10 });
    // Specific shape for skybox
    //auto skybox = std::make_shared<Shape>(rp3d::Vector3{ 1000, 1000, 1000 }, &skybox_mat, &skyboxshader, &m, nullptr);
    
    // Loading a sphere model
    auto sphere = std::make_shared<Model>("../sphere.obj", std::vector<Material>{ material }, &shader, &m, man.get(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
    sphere->CreateSphereShape(); // Creating sphere collision shape for a model
    sphere->SetPosition({ 10.f, 10.f, 10.f });

    auto terrain = std::make_shared<Model>("../terrain.obj", std::vector<Material>{ material }, &shader, &m, man.get(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
    terrain->CreateConcaveShape();
    terrain->SetPosition({ 10.f, -30.f, 10.f });

    SceneManager scene;
    scene.AddShape(s);
    scene.AddShape(s1);
    scene.AddShape(s2);

    scene.AddModel(sphere);
    scene.AddModel(terrain);

    scene.AddPhysicsManager(man);

    scene.AddLight(&l);
    
    scene.SetCamera(&cam);

    //scene.SetSkybox(skybox);
    scene.SetEnvironment(env);

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

        //skybox->SetPosition(cam.GetPosition());

        shadows.Update();
		
        scene.Draw(&buf);
        
        post.Bind();
        post.SetUniform1f("exposure", 2.0);

        buf.Draw();
    });

    // Launching the game!!!
    engine.Launch();
}
