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

float Blend(float c1, float c2, float a)
{
    return std::clamp(((c1 * (1.0 - a)) + (c2 * a)) * 255.0, 0.0, 255.0);
}

std::vector<std::vector<std::string>> ParseAnimations(std::string anims)
{
    std::vector<std::vector<std::string>> ret;
    std::vector<std::string> tmp;

    while(anims.find(",") != std::string::npos || anims.find(":") != std::string::npos)
    {
        if(anims.find(":") < anims.find(","))
        {
            tmp.push_back(anims.substr(0, anims.find(":")));
            anims.erase(0, anims.find(":") + 1);
            ret.push_back(tmp);
            tmp.clear();
        }
        if(anims.find(",") != std::string::npos)
        {
            tmp.push_back(anims.substr(0, anims.find(",")));
            anims.erase(0, anims.find(",") + 1);
        }
    }

    tmp.push_back(anims);
    ret.push_back(tmp);

    return ret;
}

int main(int argc, char* argv[])
{
    if(ArgumentExists(argc, argv, "--help"))
    {
        std::cout << "3Dev-Render is a tool for drawing a scene to an image." << std::endl
                  << "Usage: render [options] -s path/to/scene" << std::endl
                  << "  --help            Display this information" << std::endl
                  << "  -s <file>         Path to the scene file" << std::endl
                  << "  -a <name>         Name of animation" << std::endl
                  << "  -o <file>         Name for the output image/video (output.jpg/.mp4 is default)" << std::endl
                  << "  -e <file>         Path to the hdri environment (${HOME}/.3Dev-Editor/default/hdri.hdr is default)" << std::endl
                  << "  -w <int>          Width of the output (1280 is default)" << std::endl
                  << "  -h <int>          Height of the output (720 is default)" << std::endl
                  << "  -b <int>          Size of a skybox side (512 is default)" << std::endl
                  << "  -f <int>          Output video framerate (30 is default)" << std::endl
                  << "  -x <float>        Exposure (1.5 is default)" << std::endl
                  << "  -r <int>          Shadow map resolution (4096 is default)" << std::endl
                  << "  -i <int>          Blur iterations (8 is default)" << std::endl
                  << "  -n <float>        Bloom strength (0.3 is default)" << std::endl
                  << "  -t <float>        Brightness threshold (2.5 is default)" << std::endl;
        return 0;
    }

    uint32_t w = 1280, h = 720, b = 256, r = 4096, fps = 30;
    int blurIterations = 8;
    float bloomStrength = 0.3;
    float brightnessThreshold = 2.5;
    float exp = 1.0;
    #ifdef _WIN32
    	std::string env = std::string(getenv("HOMEPATH")) + "/.3Dev-Editor/default/hdri.hdr";
    #else
		std::string env = std::string(getenv("HOME")) + "/.3Dev-Editor/default/hdri.hdr";
    #endif
    std::string out = "output.jpg", scenePath, animation;

    std::vector<std::vector<std::string>> animationsQueue;

    if(!GetArgument(argc, argv, "-s").empty()) scenePath = GetArgument(argc, argv, "-s");
    else Log::Write("No path to scene given", Log::Type::Critical);

    if(!GetArgument(argc, argv, "-a").empty()) animation = GetArgument(argc, argv, "-a");
    if(!GetArgument(argc, argv, "-w").empty()) w = std::stoi(GetArgument(argc, argv, "-w"));
    if(!GetArgument(argc, argv, "-h").empty()) h = std::stoi(GetArgument(argc, argv, "-h"));
    if(!GetArgument(argc, argv, "-b").empty()) b = std::stoi(GetArgument(argc, argv, "-b"));
    if(!GetArgument(argc, argv, "-r").empty()) r = std::stoi(GetArgument(argc, argv, "-r"));
    if(!GetArgument(argc, argv, "-f").empty()) fps = std::stoi(GetArgument(argc, argv, "-f"));
    if(!GetArgument(argc, argv, "-x").empty()) exp = std::stof(GetArgument(argc, argv, "-x"));
    if(!GetArgument(argc, argv, "-i").empty()) blurIterations = std::stof(GetArgument(argc, argv, "-i"));
    if(!GetArgument(argc, argv, "-n").empty()) bloomStrength = std::stof(GetArgument(argc, argv, "-n"));
    if(!GetArgument(argc, argv, "-t").empty()) brightnessThreshold = std::stof(GetArgument(argc, argv, "-t"));
    if(!GetArgument(argc, argv, "-o").empty()) out = GetArgument(argc, argv, "-o");
    if(!GetArgument(argc, argv, "-e").empty()) env = GetArgument(argc, argv, "-e");

    if(!animation.empty())
    {
        if(out == "output.jpg")
            out = "output.mp4";
        if(std::system("ffmpeg") == 32512)
            Log::Write("You can't render animation without ffmpeg", Log::Type::Critical);

        animationsQueue = ParseAnimations(animation);
    }

    Engine engine;

    sf::Context ctx;
    ctx.setActive(true);

    engine.Init();

    Renderer::GetInstance()->Init({ w, h }, env, b, 128, b);

    Camera cam({ w, h });

    rp3d::PhysicsWorld::WorldSettings st;
    auto man = std::make_shared<PhysicsManager>(st);

    Light shadowSource({ 0, 0, 0 }, { 30.1, 50.0, -30.1 }, true);
    shadowSource.SetDirection({ 0.0, -1.0, 0.0 });

    Material skyboxMaterial(
    {
        { Renderer::GetInstance()->GetTexture(Renderer::TextureType::Skybox), Material::Type::Cubemap }
    });

    auto skybox = std::make_shared<Model>(true);
    skybox->SetMaterial({ &skyboxMaterial });

    auto path = std::filesystem::current_path();

    auto sman = std::make_shared<SoundManager>();

    SceneManager scene;

    scene.SetPhysicsManager(man);
    scene.SetCamera(&cam);
    scene.SetSkybox(skybox);
    scene.SetSoundManager(sman);
    scene.UpdatePhysics(false);

    scene.Load(scenePath, true);
    
    if(scene.GetNames()[2].empty())
        scene.AddLight(&shadowSource);

    std::filesystem::current_path(path);

    ShadowManager shadows(&scene, glm::ivec2(r, r));

    std::vector<Framebuffer*> pingPongBuffers = 
    {
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong0),
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong1)
    };

    Framebuffer render(nullptr, w, h), renderTr(nullptr, w, h);

    sf::Image image;

    auto draw = [&]()
    {
        cam.Look();

        shadows.Update();

        scene.Draw();

        bool horizontal = true;
        bool buffer = true;

        if(bloomStrength > 0)
        {
            pingPongBuffers[0]->Bind();
            Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->Bind();
            Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1f("brightnessThreshold", brightnessThreshold);
            Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("rawColor", true);
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->Draw();

            for(int i = 0; i < blurIterations; i++)
            {
                pingPongBuffers[buffer]->Bind();
                Renderer::GetInstance()->GetShader(Renderer::ShaderType::Bloom)->Bind();
                Renderer::GetInstance()->GetShader(Renderer::ShaderType::Bloom)->SetUniform1i("horizontal", horizontal);
                pingPongBuffers[!buffer]->Draw();
                buffer = !buffer; horizontal = !horizontal;
            }
        }

        render.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_2D, pingPongBuffers[buffer]->GetTexture());
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->Bind();
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1f("exposure", exp);
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1f("bloomStrength", bloomStrength);
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("bloom", 15);
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("rawColor", false);
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->Draw();
        auto pixels = render.GetPixels(glm::ivec2(0, 0), render.GetSize());

        renderTr.Bind();
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Transparency)->Draw();
        auto pixelsTr = renderTr.GetPixels(glm::ivec2(0, 0), renderTr.GetSize());

        image.create({ w, h });

        for(int i = 0; i < h; i++)
            for(int j = 0; j < w; j++)
            {
                float a = pixelsTr[((i * w + j) * 4) + 3];
                float r = Blend(pixels[(i * w + j) * 4], std::isnan(pixelsTr[(i * w + j) * 4]) ? 0 : pixelsTr[(i * w + j) * 4], a);
                float g = Blend(pixels[((i * w + j) * 4) + 1], std::isnan(pixelsTr[((i * w + j) * 4) + 1]) ? 0 : pixelsTr[((i * w + j) * 4) + 1], a);
                float b = Blend(pixels[((i * w + j) * 4) + 2], std::isnan(pixelsTr[((i * w + j) * 4) + 2]) ? 0 : pixelsTr[((i * w + j) * 4) + 2], a);
                sf::Color color((uint8_t)r, (uint8_t)g, (uint8_t)b);
                image.setPixel({ j, h - 1 - i }, color);
            }

        free(pixels);
        free(pixelsTr);
    };

    if(animation.empty())
    {
        draw();
        image.saveToFile(out);
        exit(0);
    }

    int tmpCount = 0;
    for(auto& i : animationsQueue)
    {
        std::shared_ptr<Animation> maxDuration = scene.GetAnimation(i[0]);
        std::vector<std::shared_ptr<Animation>> playingAnims;

        for(auto& j : i)
        {
            std::shared_ptr<Animation> anim = scene.GetAnimation(j);
            if(!anim)
                Log::Write("Can't find an animation with name \"" + j + "\"", Log::Type::Critical);
            anim->Pause();
            anim->SetLastTime(0);
            playingAnims.push_back(anim);
            if((anim->GetDuration() / anim->GetTPS() > maxDuration->GetDuration() / maxDuration->GetTPS()) && !anim->IsRepeated())
                maxDuration = anim;
        }

        while(maxDuration->GetLastTime() < maxDuration->GetDuration())
        {
            draw();

            image.saveToFile("temp" + std::to_string(tmpCount) + ".jpg");

            for(auto j : playingAnims)
            {
                j->SetLastTime(j->GetLastTime() + (1.0 / float(fps)) * j->GetTPS());
                if(j->GetLastTime() > j->GetDuration())
                {
                    if(j->IsRepeated())
                        j->SetLastTime(0);
                    else j->SetLastTime(j->GetDuration());
                }
            }

            tmpCount++;
        }
        
        for(auto j : playingAnims) j->Stop();
    }

    std::system(std::string("ffmpeg -f image2 -r " + std::to_string(fps) + " -i temp%d.jpg " + out).c_str());
    for(int i = 0; i < tmpCount; i++)
        std::filesystem::remove("temp" + std::to_string(i) + ".jpg");
}
