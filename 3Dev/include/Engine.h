#include "3Dev.h"
#include "Camera.h"
#include "Model.h"
#include "Light.h"
#include "Shape.h"
#include "Shader.h"
#include "Matrices.h"
#include "Framebuffer.h"
#include "PhysicsManager.h"
#include <functional>

class Engine
{
public:
    Engine() {}
    Engine(sf::ContextSettings& settings) : settings(settings) {}

    void Init();
    void Init(std::function<void(void)> init);

    void CreateWindow(float width, float height, std::string title, sf::Uint32 style = sf::Style::Default);

    void Loop(std::function<void(void)> loop);
    void EventLoop(std::function<void(sf::Event&)> eloop);

    void Launch();
    void Close();

    sf::RenderWindow& GetWindow();
    sf::ContextSettings& GetSettings();

private:
    bool running = false;

    std::function<void(void)> loop;
    std::function<void(sf::Event&)> eloop;

    sf::ContextSettings settings;
    sf::RenderWindow window;
    sf::Event event;
};
