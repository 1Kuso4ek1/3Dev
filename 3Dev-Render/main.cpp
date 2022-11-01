#include <Engine.hpp>

std::string GetArgument(int argc, char* argv[], std::string arg)
{
    auto it = std::find(argv, argv + argc, arg);
    if(it != argv + argc && ++it != argv + argc)
        return std::string(*it);
    return "";
}

bool ArgumentExists(int argc, char* argv[], std::string arg)
{
    return std::find(argv, argv + argc, arg) != argv + argc;
}

int main(int argc, char* argv[])
{
    if(ArgumentExists(argc, argv, "--help"))
    {
        std::cout << "3Dev-Render is a tool for drawing a scene to an image." << std::endl
                  << "Usage: render [options] -s path/to/scene" << std::endl
                  << "  --help            Display this information" << std::endl
                  << "  -s <file>         Path to the scene file" << std::endl
                  << "  -o <file>         Name for the output image (output.png is default)" << std::endl
                  << "  -e <file>         Path to the hdri environment (${HOME}/.3Dev-Editor/default/hdri.hdr is default)" << std::endl
                  << "  -w <number>       Width of the output (1280 is default)" << std::endl
                  << "  -h <number>       Height of the output (720 is default)" << std::endl
                  << "  -b <number>       Size of a skybox side (1024 is default)" << std::endl
                  << "  -x <number>       Exposure (1.5 is default)" << std::endl;
        return 0;
    }

    uint32_t w = 1280, h = 720, b = 1024;
    float exp = 1.5;
    std::string env = std::string(getenv("HOME")) + "/.3Dev-Editor/default/hdri.hdr", out = "output.png", scenePath;

    if(!GetArgument(argc, argv, "-s").empty()) scenePath = GetArgument(argc, argv, "-s");
    else Log::Write("No path to scene given", Log::Type::Critical);

    if(!GetArgument(argc, argv, "-w").empty()) w = std::stoi(GetArgument(argc, argv, "-w"));
    if(!GetArgument(argc, argv, "-h").empty()) h = std::stoi(GetArgument(argc, argv, "-h"));
    if(!GetArgument(argc, argv, "-b").empty()) b = std::stoi(GetArgument(argc, argv, "-b"));
    if(!GetArgument(argc, argv, "-x").empty()) exp = std::stof(GetArgument(argc, argv, "-x"));
    if(!GetArgument(argc, argv, "-o").empty()) out = GetArgument(argc, argv, "-o");
    if(!GetArgument(argc, argv, "-e").empty()) env = GetArgument(argc, argv, "-e");

    Engine engine;

    sf::Context ctx;
    ctx.setActive(true);

    engine.Init();

    Renderer::GetInstance()->Init({ w, h }, env, b, 128, b);

    Camera cam({ w, h });

    rp3d::PhysicsWorld::WorldSettings st;
    auto man = std::make_shared<PhysicsManager>(st);

    Light shadowSource({ 0, 0, 0 }, { 50.1, 100.0, 50.1 });
    shadowSource.SetDirection({ 0.0, -1.0, 0.0 });

    Material skyboxMaterial(
    {
        { Renderer::GetInstance()->GetTexture(Renderer::TextureType::Skybox), Material::Type::Cubemap }
    });

    auto skybox = std::make_shared<Shape>(rp3d::Vector3{ 1, 1, 1 }, &skyboxMaterial);

    auto path = std::filesystem::current_path();

    SceneManager scene;

    scene.AddPhysicsManager(man);
    scene.SetCamera(&cam);
    scene.SetSkybox(skybox);
    scene.UpdatePhysics(false);

    scene.Load(scenePath);

    std::filesystem::current_path(path);

    ShadowManager shadows(&scene, { &shadowSource }, glm::ivec2(4096, 4096));

    Framebuffer render(nullptr, w, h);

    cam.Look();

    shadows.Update();

    scene.Draw();

    Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->Bind();
    Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1f("exposure", exp);

    render.Bind();

    Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->Draw();
    //Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Transparency)->Draw();

    sf::Image image;
    image.create(w, h);

    auto pixels = render.GetPixels(glm::ivec2(0, 0), render.GetSize());
    for(int i = 0; i < h; i++)
        for(int j = 0; j < w; j++)
        {
            float r = pixels[(i * w + j) * 4] * 255; if(r > 255) r = 255;
            float g = pixels[((i * w + j) * 4) + 1] * 255; if(g > 255) g = 255;
            float b = pixels[((i * w + j) * 4) + 2] * 255; if(b > 255) b = 255;

            sf::Color color((uint8_t)r, (uint8_t)g, (uint8_t)b);
            image.setPixel(j, h - 1 - i, color);
        }

    image.saveToFile(out);
}
