#define DISABLE_MULTITHREADING
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
                  << "  -x <float>        Exposure (1.0 is default)" << std::endl
                  << "  -r <int>          Shadow map resolution (4096 is default)" << std::endl
                  << "  -i <int>          Blur iterations (8 is default)" << std::endl
                  << "  -n <float>        Bloom strength (0.3 is default)" << std::endl
                  << "  -c <float>        Bloom resolution scale (10 is default)" << std::endl
                  << "  -l <float>        SSAO strength (2.0 is default)" << std::endl
                  << "  -d <float>        SSAO radius (0.5 is default)" << std::endl
                  << "  -m <int>          SSAO samples (64 is default)" << std::endl
                  << "  -t <float>        DOF min distance (1.0 is default)" << std::endl
                  << "  -g <float>        DOF max distance (1.0 is default)" << std::endl
                  << "  -u <float>        DOF focus distance (1.0 is default)" << std::endl
                  << "  -v <float>        Fog start (0.0 is default)" << std::endl
                  << "  -k <float>        Fog end (0.0 is default)" << std::endl
                  << "  -p <float>        Fog height (0.0 is default)" << std::endl;
        return 0;
    }

    uint32_t w = 1280, h = 720, b = 256, r = 4096, fps = 30;
    int blurIterations = 8;
    int ssaoSamples = 64;
    float bloomStrength = 0.3;
    float exposure = 1.0;
    float ssaoStrength = 2.0;
    float ssaoRadius = 0.5;
    float bloomResolutionScale = 10.0;
    float dofMinDistance = 1.0;
    float dofMaxDistance = 1.0;
    float dofFocusDistance = 1.0;
    float fogStart = 0.0;
    float fogEnd = 0.0;
    float fogHeight = 0.0;
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
    if(!GetArgument(argc, argv, "-x").empty()) exposure = std::stof(GetArgument(argc, argv, "-x"));
    if(!GetArgument(argc, argv, "-i").empty()) blurIterations = std::stof(GetArgument(argc, argv, "-i"));
    if(!GetArgument(argc, argv, "-n").empty()) bloomStrength = std::stof(GetArgument(argc, argv, "-n"));
    if(!GetArgument(argc, argv, "-c").empty()) bloomResolutionScale = std::stof(GetArgument(argc, argv, "-c"));
    if(!GetArgument(argc, argv, "-o").empty()) out = GetArgument(argc, argv, "-o");
    if(!GetArgument(argc, argv, "-e").empty()) env = GetArgument(argc, argv, "-e");
    if(!GetArgument(argc, argv, "-l").empty()) ssaoStrength = std::stof(GetArgument(argc, argv, "-l"));
    if(!GetArgument(argc, argv, "-d").empty()) ssaoRadius = std::stof(GetArgument(argc, argv, "-d"));
    if(!GetArgument(argc, argv, "-m").empty()) ssaoSamples = std::stof(GetArgument(argc, argv, "-m"));
    if(!GetArgument(argc, argv, "-t").empty()) dofMinDistance = std::stof(GetArgument(argc, argv, "-t"));
    if(!GetArgument(argc, argv, "-g").empty()) dofMaxDistance = std::stof(GetArgument(argc, argv, "-g"));
    if(!GetArgument(argc, argv, "-u").empty()) dofFocusDistance = std::stof(GetArgument(argc, argv, "-u"));
    if(!GetArgument(argc, argv, "-v").empty()) fogStart = std::stof(GetArgument(argc, argv, "-v"));
    if(!GetArgument(argc, argv, "-k").empty()) fogEnd = std::stof(GetArgument(argc, argv, "-k"));
    if(!GetArgument(argc, argv, "-p").empty()) fogHeight = std::stof(GetArgument(argc, argv, "-p"));

    if(!animation.empty())
    {
        if(out == "output.jpg")
            out = "output.mp4";

        animationsQueue = ParseAnimations(animation);
    }

    Multithreading::GetInstance()->SetIsEnabled(false);
    
    Engine engine;

    sf::Context ctx;
    ctx.setActive(true);

    engine.Init();

    Renderer::GetInstance()->SetSSAOSamples(ssaoSamples);
    Renderer::GetInstance()->SetIsSSREnabled(true);
    Renderer::GetInstance()->SetSSRRayStep(0.005);
    Renderer::GetInstance()->SetSSRMaxSteps(500);
    Renderer::GetInstance()->SetSSRMaxBinarySearchSteps(100);
    Renderer::GetInstance()->Init({ w, h }, env, b, 128, b, bloomResolutionScale);

    Camera cam({ w, h });

    rp3d::PhysicsWorld::WorldSettings st;
    auto man = std::make_shared<PhysicsManager>(st);

    auto shadowSource = std::make_shared<Light>(rp3d::Vector3(0, 0, 0), rp3d::Vector3(50.1, 100.0, 50.1), true);

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
        scene.AddLight(shadowSource);

    std::filesystem::current_path(path);

    ShadowManager shadows(&scene, glm::ivec2(r, r));

    std::vector<Framebuffer*> pingPongBuffers = 
    {
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong0),
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong1)
    };

    std::vector<Framebuffer*> pingPongBuffers1 = 
    {
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong2),
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong3)
    };

    Framebuffer render(nullptr, w, h), renderTr(nullptr, w, h);

    sf::Image image;

    Renderer::GetInstance()->SetExposure(exposure);
    Renderer::GetInstance()->SetBloomStrength(bloomStrength);
    Renderer::GetInstance()->SetBlurIterations(blurIterations);
    Renderer::GetInstance()->SetSSAOStrength(ssaoStrength);
    Renderer::GetInstance()->SetSSAORadius(ssaoRadius);
    Renderer::GetInstance()->SetDOFMinDistance(dofMinDistance);
    Renderer::GetInstance()->SetDOFMaxDistance(dofMaxDistance);
    Renderer::GetInstance()->SetDOFFocusDistance(dofFocusDistance);
    Renderer::GetInstance()->SetFogStart(fogStart);
    Renderer::GetInstance()->SetFogEnd(fogEnd);
    Renderer::GetInstance()->SetFogHeight(fogHeight);

    auto draw = [&]()
    {
        cam.Look();

        shadows.Update();

        scene.Draw();

        render.Bind();
        auto size = Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->GetSize();
        glViewport(0, 0, size.x, size.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_2D, pingPongBuffers[blurIterations % 2 == 0]->GetTexture());
        glActiveTexture(GL_TEXTURE16);
        glBindTexture(GL_TEXTURE_2D, Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::SSR)->GetTexture());
        glActiveTexture(GL_TEXTURE17);
        glBindTexture(GL_TEXTURE_2D, Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::SSGIPingPong1)->GetTexture());
        glActiveTexture(GL_TEXTURE18);
        glBindTexture(GL_TEXTURE_2D, pingPongBuffers1[blurIterations % 2 == 0]->GetTexture());
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->Bind();
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1f("exposure", exposure);
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1f("bloomStrength", bloomStrength);
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("bloom", 15);
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("ssr", 16);
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("ssgi", 17);
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("bloom1", 18);
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("rawColor", false);
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("transparentBuffer", false);
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->Draw();
        auto pixels = render.GetPixels(glm::ivec2(0, 0), render.GetSize());

        renderTr.Bind();
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->Bind();
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("transparentBuffer", true);
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
