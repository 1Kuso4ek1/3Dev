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

/*
 * This class is responsible for creating a window, handling events and launching game loop
 */
class Engine
{
public:
    // Default constructor
    Engine() {}

    // @param settings SFML window settings 
    Engine(sf::ContextSettings& settings) : settings(settings) {}

    // Default OpenGL initialization function
    void Init();

    /*
     * Custom initialization function
     * @param init your custom function
     */
    void Init(std::function<void(void)> init);

    /*
     * Function for creating SFML window
     * @param width width of your window
     * @param height height of your window
     * @param title title of your window
     * @param style SFML window style (sf::Style::None, sf::Style::Titlebar, sf::Style::Resize, sf::Style::Close, sf::Style::Fullscreen, sf::Style::Default)
     */
    void CreateWindow(float width, float height, std::string title, sf::Uint32 style = sf::Style::Default);

    /*
     * Set main game loop
     * @param loop function, that will execute every frame
     */
    void Loop(std::function<void(void)> loop);

    /*
     * Set function for handling SFML window events
     * @param eloop event handling function
     */
    void EventLoop(std::function<void(sf::Event&)> eloop);

    // Launch your game
    void Launch();

    // Close your game
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