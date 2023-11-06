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
        Deffered,
        Forward,
        LightingPass,
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
        GBuffer,
        Main,
        Transparency,
        BloomPingPong0,
        BloomPingPong1
    };

    static Renderer* GetInstance();
    static void DeleteInstance();

    void Init(sf::Vector2u fbSize, const std::string& environmentMapFilename, uint32_t skyboxSideSize = 256, uint32_t irradianceSideSize = 32, uint32_t prefilteredSideSize = 256, float bloomResolutionScale = 10);
    void LoadEnvironment(const std::string& environmentMapFilename);
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

    std::shared_ptr<Framebuffer> capture, captureIrr, captureSpc, captureBRDF;

    std::unordered_map<TextureType, GLuint> textures;
    std::unordered_map<ShaderType, std::shared_ptr<Shader>> shaders;
    std::unordered_map<FramebufferType, std::shared_ptr<Framebuffer>> framebuffers;
};
