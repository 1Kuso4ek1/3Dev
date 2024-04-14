#pragma once
#include "Utils.hpp"
#include "Multithreading.hpp"
#include "Node.hpp"
#include "Camera.hpp"
#include "Model.hpp"
#include "Light.hpp"
#include "Shader.hpp"
#include "Matrices.hpp"
#include "Framebuffer.hpp"
#include "PhysicsManager.hpp"
#include "SceneManager.hpp"
#include "ShadowManager.hpp"
#include "SoundManager.hpp"
#include "Renderer.hpp"
#include "ScriptManager.hpp"
#include "TextureManager.hpp"

/*
 * This class is responsible for creating a window, handling events and launching game loop
 */
class Engine
{
public:
    // Default constructor
    Engine(bool initLog = true, bool silentLog = false);

    // Default OpenGL initialization function
    void Init();

    /*
     * Function for creating SFML window
     * @param width width of your window
     * @param height height of your window
     * @param title title of your window
     * @param style SFML window style (sf::Style::None, sf::Style::Titlebar, sf::Style::Resize, sf::Style::Close, sf::Style::Fullscreen, sf::Style::Default)
     */
    void CreateWindow(uint32_t width, uint32_t height, std::string title, uint32_t style = sf::Style::Default);

    tgui::Gui* CreateGui(std::string widgets);

    void RemoveGui();

    void SetGuiView(tgui::FloatRect view);
    void SetGuiViewport(tgui::FloatRect viewport);

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
    void Launch(bool handleResize = true);

    // Close your game
    void Close();

    sf::Window& GetWindow();
    sf::ContextSettings& GetSettings();

private:
    bool running = false;

    std::function<void(void)> loop;
    std::function<void(sf::Event&)> eloop;

    std::vector<std::shared_ptr<tgui::Gui>> gui;
    tgui::FloatRect view, viewport;

    sf::ContextSettings settings;
    sf::Window window;
    sf::Event event;
};
