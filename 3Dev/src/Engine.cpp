#include "Engine.hpp"

Engine::Engine(bool initLog, bool silentLog)
{
    if(initLog)
        Log::Init("3Dev_log.txt", silentLog);
}

Engine::~Engine()
{
    Renderer::DeleteInstance();
    TextureManager::DeleteInstance();
}

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
}

void Engine::CreateWindow(float width, float height, std::string title, sf::Uint32 style)
{
    window.create(sf::VideoMode(width, height), title, style, settings);
}

tgui::Gui* Engine::CreateGui(std::string widgets)
{
    gui.emplace_back(std::make_shared<tgui::Gui>(window));
    gui.back()->loadWidgetsFromFile(widgets);
    gui.back()->setAbsoluteViewport(viewport);
    return gui.back().get();
}

void Engine::RemoveGui()
{
    gui.clear();
}

void Engine::SetGuiViewport(tgui::FloatRect viewport)
{
    this->viewport = viewport;
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
            for(auto i : gui)
                i->handleEvent(event);
            if(event.type == sf::Event::Resized)
                glViewport(0, 0, (float)event.size.width, (float)event.size.height);
        }

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        try
        {
            loop();

            for(auto i : gui)
                i->draw();
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
}

void Engine::Close()
{
    running = false;
}

sf::Window& Engine::GetWindow()
{
    return window;
}

sf::ContextSettings& Engine::GetSettings()
{
    return settings;
}
