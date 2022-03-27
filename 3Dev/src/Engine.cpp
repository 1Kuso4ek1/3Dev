#include "Engine.h"

void Engine::Init()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0);
    glDepthMask(GL_TRUE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);
    glewInit();
}

void Engine::Init(std::function<void(void)> init)
{
    init();
}

void Engine::CreateWindow(float width, float height, std::string title, sf::Uint32 style)
{
    window.create(sf::VideoMode(width, height), title, style, settings);
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
    running = true;

    while(running)
    {
        while(window.pollEvent(event))
        {
            eloop(event);
            if(event.type == sf::Event::Resized)
            {
                glViewport(0, 0, (float)event.size.width, (float)event.size.height);
            }
        }

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        loop();

        window.display();
    }
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