#include <Engine.h>

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
    std::vector<std::string> skybox_textures = 
    {
        "../textures/skybox_right.bmp",
        "../textures/skybox_left.bmp",
        "../textures/skybox_top.bmp",
        "../textures/skybox_bottom.bmp",
        "../textures/skybox_front.bmp",
        "../textures/skybox_back.bmp"
    };
    GLuint cubemap = LoadCubemap(skybox_textures);

    // Textures for a material
    GLuint texture = LoadTexture("../textures/metal_color.jpg"), normalmap = LoadTexture("../textures/metal_normal.jpg"),
    ao = LoadTexture("../textures/metal_ao.jpg"), metalness = LoadTexture("../textures/metal_metalness.jpg"),
    roughness = LoadTexture("../textures/metal_roughness.jpg");

    /*GLuint cmakarov = LoadTexture("textures/makarov_color.png"), nmakarov = LoadTexture("textures/makarov_normal.png"),
    rmakarov = LoadTexture("textures/makarov_roughness.png"), mmakarov = LoadTexture("textures/makarov_metalness.png");

    GLuint csight = LoadTexture("textures/sight_color.png"), nsight = LoadTexture("textures/sight_normal.png"),
    rsight = LoadTexture("textures/sight_roughness.png"), msight = LoadTexture("textures/sight_metalness.png"),
    osight = LoadTexture("textures/sight_opacity.png");

    GLuint csuppressor = LoadTexture("textures/suppressor_color.jpg"), nsuppressor = LoadTexture("textures/suppressor_normal.jpg"),
    rsuppressor = LoadTexture("textures/suppressor_roughness.jpg"), msuppressor = LoadTexture("textures/suppressor_metalness.jpg");*/

    rp3d::PhysicsWorld::WorldSettings st; // Default physics world settings
    PhysicsManager man(st); // Main physics manager

    Shader skyboxshader("../shaders/skybox.vs", "../shaders/skybox.frag"); // Shader for skybox rendering
    Shader shader("../shaders/vertex.vs", "../shaders/fragment.frag"); // Main shader
    Shader post("../shaders/post.vs", "../shaders/post.frag"); // Post-processing shader
    Framebuffer buf(&post, 1280, 720); // Main framebuffer

    // Function for handling SFML window events
    engine.EventLoop([&](sf::Event& event)
    {
        if(event.type == sf::Event::Resized) // If the window is resized
            buf.RecreateTexture(event.size.width, event.size.height); // Resizing framebuffer texture

        if(event.type == sf::Event::Closed) engine.Close(); // Closing the window
    });

    // Main light
    Light l({ 0.1, 0.1, 0.1 }, { 1, 1, 1 }, { 25, 25, 25 }, { 0.0, 50.0, 0.0 });
    l.SetAttenuation(0.0, 0.1, 0.0);

    // Main material
    Material material(32,
    { 
    	{ texture, Material::TexType::Diffuse },
    	{ normalmap, Material::TexType::NormalMap },
    	{ metalness, Material::TexType::Metalness },
    	{ ao, Material::TexType::AmbientOcclusion },
    	{ roughness, Material::TexType::Roughness },
    	{ cubemap, Material::TexType::Cubemap }
    });
    /*Material makarov_material(32, { { cmakarov, Material::TexType::Diffuse }, { nmakarov, Material::TexType::NormalMap }, { mmakarov, Material::TexType::Metalness }, { rmakarov, Material::TexType::Roughness }, { cubemap, Material::TexType::Cubemap } });
    Material sight_material(32, { { csight, Material::TexType::Diffuse }, { nsight, Material::TexType::NormalMap }, { msight, Material::TexType::Metalness }, { rsight, Material::TexType::Roughness }, { osight, Material::TexType::Opacity }, { cubemap, Material::TexType::Cubemap } });
    Material suppressor_material(32, { { csuppressor, Material::TexType::Diffuse }, { nsuppressor, Material::TexType::NormalMap }, { msuppressor, Material::TexType::Metalness }, { rsuppressor, Material::TexType::Roughness }, { cubemap, Material::TexType::Cubemap } });*/
    
    // All the shapes
    Shape s({ 10, 30, 10 }, { 3, 3, 3 }, rp3d::Quaternion::identity(), &man, &shader, &material, &m);
    //Shape s1({ 0, -10, 0 }, { 10, 2, 30 }, rp3d::Quaternion::fromEulerAngles(0.348, 0, 0), &man, &shader, &material, &m);
    //Shape s2({ 60, -100, 60 }, { 100, 10, 100 }, rp3d::Quaternion::identity(), &man, &shader, &material, &m);
    Shape s3({ 10, 36, 10 }, { 3, 3, 3 }, rp3d::Quaternion::identity(), &man, &shader, &material, &m);
    Shape s4({ 10, 13, 10 }, { 1.5, 1.5, 3 }, rp3d::Quaternion::fromEulerAngles(0.0, 0.0, 0.0), &man, &shader, &material, &m);
    // Specific shape for skybox
    Shape skybox({ 0, 0, 0 }, { 1000, 1000, 1000 }, rp3d::Quaternion::identity(), nullptr, &skyboxshader, nullptr, &m, cubemap);
    
    // Loading a sphere model
    Model sphere("../sphere.obj", { material }, &shader, &m, &man, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes, { 10.f, 10.f, 10.f });
    sphere.CreateSphereShape(); // Creating sphere collision shape for a model

    Model terrain("../terrain.obj", { material }, &shader, &m, &man, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes, { 10.f, -30.f, 10.f });
    terrain.CreateConcaveShape();

    //Model makarov("makarov.obj", { makarov_material, suppressor_material, sight_material, }, &shader, &m, nullptr, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes, { 10.f, 50.f, 10.f });

    // Some shapes should be static
    //s1.GetRigidBody()->setType(rp3d::BodyType::STATIC);
    //s2.GetRigidBody()->setType(rp3d::BodyType::STATIC);

    bool launched = false; // If true, physics simulation is started
    sf::Clock clock, global;
    
    // Main game loop
    engine.Loop([&]() 
    {
        float delta_time = clock.restart().asSeconds();

        // Launching physics simulation when Q key is pressed
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) launched = true;
        // Camera movement, rotation and so on
        cam.Update();
        cam.Move(1);
        cam.Mouse();
        cam.Look();
        //////////////////////////////////////

        // Updating simulation
        if(launched)
            do
            {
                man.Update(1.0 / 60.0);
                delta_time -= 1.0 / 60.0;
            } while (delta_time >= 1.0 / 60.0);

        // Skybox is moving with the camera
        skybox.SetPosition(cam.GetPosition());

        // Binding framebuffer
        buf.Bind();

        // Rendering all shapes
        s.Draw(cam, { l });
        //s1.Draw(cam, { l });
        //s2.Draw(cam, { l });
        s3.Draw(cam, { l });
        s4.Draw(cam, { l });
        /////////////////////

        // Rendering sphere model
        sphere.Draw(cam, { l });
        terrain.Draw(cam, { l });
        //makarov.Draw(cam, { l });

        // Rendering skybox
        skybox.DrawSkybox();

        // Unbinding framebuffer
        buf.Unbind();
        post.Bind();
        post.SetUniform1f("exposure", 2.0);
        // Drawing framebuffer texture on the screen
        buf.Draw();
    });

    // Launching the game!!!
    engine.Launch();
}
