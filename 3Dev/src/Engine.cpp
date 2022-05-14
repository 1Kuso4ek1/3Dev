#include "Engine.hpp"

void Engine::Init()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0);
    glEnable(GL_MULTISAMPLE);
    glDepthMask(GL_TRUE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glewInit();
    Log::Write("Engine successfully initialized", Log::Type::Info);
}

void Engine::CreateWindow(float width, float height, std::string title, sf::Uint32 style)
{
    Log::Init("3Dev_log.txt", false);
    window.create(sf::VideoMode(width, height), title, style, settings);
    Log::Write("Window successfully created", Log::Type::Info);
}

void Engine::EventLoop(std::function<void(sf::Event&)> eloop)
{
    this->eloop = eloop;
}

void Engine::Loop(std::function<void(void)> loop)
{
    this->loop = loop;
}

void Engine::Launch()
{
    Log::Write("Engine launched", Log::Type::Info);

    running = true;

    while(running)
    {
        while(window.pollEvent(event))
        {
            eloop(event);
            if(event.type == sf::Event::Resized)
                glViewport(0, 0, (float)event.size.width, (float)event.size.height);
        }

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        try
        {
            loop();
        }
        catch(std::exception& e)
        {
            Log::Write("Exception catched! e.what():" + std::string(e.what()), Log::Type::Critical);
        }
        catch(...)
        {
            Log::Write("Unknown exception catched!", Log::Type::Critical);
        }

        window.display();
    }

    Log::Write("Engine closed", Log::Type::Info);
}

void Engine::Close()
{
    running = false;
}

sf::RenderWindow& Engine::GetWindow()
{
    return window;
}

sf::ContextSettings& Engine::GetSettings()
{
    return settings;
}