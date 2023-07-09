#pragma once
#include "Utils.hpp"
#include "Framebuffer.hpp"

class Renderer
{
public:
    enum class TextureType
    {
        Skybox,
        Irradiance,
        Prefiltered,
        LUT
    };

    enum class ShaderType
    {
        Main,
        Deffered,
        Skybox,
        Depth,
        Post,
        Environment,
        Irradiance,
        Filtering,
        BRDF,
        Bloom
    };

    enum class FramebufferType
    {
        Main,
        Transparency,
        GBuffer,
        BloomPingPong0,
        BloomPingPong1
    };

    static Renderer* GetInstance();
    static void DeleteInstance();

    void Init(sf::Vector2u fbSize, std::string environmentMapFilename, uint32_t skyboxSideSize = 256, uint32_t irradianceSideSize = 32, uint32_t prefilteredSideSize = 256);
    void LoadEnvironment(std::string environmentMapFilename, uint32_t skyboxSideSize = 256, uint32_t irradianceSideSize = 32, uint32_t prefilteredSideSize = 256);
    void SetShadersDirectory(std::string dir);

    GLuint GetTexture(TextureType type);
    Shader* GetShader(ShaderType type);
    Framebuffer* GetFramebuffer(FramebufferType type);
    Matrices* GetMatrices();

private:
    Renderer() {}

    static Renderer* instance;

    std::string shadersDir;

    Matrices m;

    std::unordered_map<TextureType, GLuint> textures;
    std::unordered_map<ShaderType, std::shared_ptr<Shader>> shaders;
    std::unordered_map<FramebufferType, std::shared_ptr<Framebuffer>> framebuffers;
};
