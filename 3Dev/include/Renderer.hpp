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
        LUT,
        Noise
    };

    enum class ShaderType
    {
        Deferred,
        Forward,
        LightingPass,
        SSAO,
        Decals,
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
        SSAO,
        DecalsGBuffer,
        BloomPingPong0,
        BloomPingPong1
    };

    static Renderer* GetInstance();
    static void DeleteInstance();

    void Init(sf::Vector2u fbSize, const std::string& environmentMapFilename, uint32_t skyboxSideSize = 256,
              uint32_t irradianceSideSize = 32, uint32_t prefilteredSideSize = 256, float bloomResolutionScale = 10,
              bool useRGBA16F = false);
    void LoadEnvironment(const std::string& environmentMapFilename);
    void SetShadersDirectory(std::string dir);

    void SetExposure(float exposure);

    void SetSSAOStrength(float strength);
    void SetSSAORadius(float radius);
    void SetSSAOSamples(int samples);

    void SetDOFMinDistance(float dist);
    void SetDOFMaxDistance(float dist);
    void SetDOFFocusDistance(float dist);
    
    void SetBloomStrength(float strength);
    void SetBlurIterations(int iterations);

    void Bloom();
    void SSAO();

    void DrawFramebuffers();

    float GetExposure();

    float GetSSAOStrength();
    float GetSSAORadius();
    int GetSSAOSamples();

    float GetDOFMinDistance();
    float GetDOFMaxDistance();
    float GetDOFFocusDistance();
    
    float GetBloomStrength();
    int GetBlurIterations();

    GLuint GetTexture(TextureType type);
    Shader* GetShader(ShaderType type);
    Framebuffer* GetFramebuffer(FramebufferType type);
    Matrices* GetMatrices();

private:
    Renderer() {}

    static Renderer* instance;

    float exposure = 1.0;
    float ssaoStrength = 2.0;
    float ssaoRadius = 0.5;
    float bloomStrength = 0.3;
    float dofMinDistance = 1.0;
    float dofMaxDistance = 1.0;
    float dofFocusDistance = 1.0;
    int blurIterations = 8;
    int samples = 64;

    bool horizontal = true;
    bool buffer = true;

    std::string shadersDir;

    Matrices m;

    std::vector<glm::vec3> ssaoSamples, noise;
    std::vector<Framebuffer*> pingPongBuffers;

    std::shared_ptr<Framebuffer> capture, captureIrr, captureSpc, captureBRDF;

    std::unordered_map<TextureType, GLuint> textures;
    std::unordered_map<ShaderType, std::shared_ptr<Shader>> shaders;
    std::unordered_map<FramebufferType, std::shared_ptr<Framebuffer>> framebuffers;
};
