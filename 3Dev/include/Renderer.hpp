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
        SSGI,
        SSR,
        Fog,
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
        SSGI,
        SSR,
        Fog,
        DecalsGBuffer,
        BloomPingPong0,
        BloomPingPong1,
        SSGIPingPong0,
        SSGIPingPong1
    };

    static Renderer* GetInstance();
    static void DeleteInstance();

    void Init(sf::Vector2u fbSize, const std::string& environmentMapFilename, uint32_t skyboxSideSize = 256,
              uint32_t irradianceSideSize = 32, uint32_t prefilteredSideSize = 256, float bloomResolutionScale = 10,
              bool useRGBA16F = false);
    void LoadEnvironment(const std::string& environmentMapFilename);
    void SetShadersDirectory(std::string dir);

    void SetSkyboxResolution(uint32_t res);
    void SetIrradianceResolution(uint32_t res);
    void SetPrefilteredResolution(uint32_t res);

    void SetExposure(float exposure);

    void SetSSAOStrength(float strength);
    void SetSSAORadius(float radius);
    void SetSSAOSamples(int samples);

    void SetDOFMinDistance(float dist);
    void SetDOFMaxDistance(float dist);
    void SetDOFFocusDistance(float dist);
    
    void SetBloomStrength(float strength);
    void SetBlurIterations(int iterations);

    void SetFogStart(float start);
    void SetFogEnd(float end);
    void SetFogHeight(float height);
    void SetFogIntensity(float intensity);

    void SetIsSSREnabled(bool ssrEnabled);

    void SetSSRRayStep(float step);
    void SetSSRMaxSteps(int steps);
    void SetSSRMaxBinarySearchSteps(int steps);

    void SetIsSSGIEnabled(bool ssgiEnabled);

    void SetSSGIStrength(float ssgiStrength);

    void Bloom();
    void SSAO();
    void SSGI();
    void SSR();
    void Fog(rp3d::Vector3 camPos);

    void DrawFramebuffers();

    uint32_t GetSkyboxResolution();
    uint32_t GetIrradianceResolution();
    uint32_t GetPrefilteredResolution();

    float GetExposure();

    float GetSSAOStrength();
    float GetSSAORadius();
    int GetSSAOSamples();

    float GetDOFMinDistance();
    float GetDOFMaxDistance();
    float GetDOFFocusDistance();

    float GetFogStart();
    float GetFogEnd();
    float GetFogHeight();
    float GetFogIntensity();
    
    float GetBloomStrength();
    int GetBlurIterations();

    bool IsSSREnabled();

    float GetSSRRayStep();
    int GetSSRMaxSteps();
    int GetSSRMaxBinarySearchSteps();

    bool IsSSGIEnabled();

    float GetSSGIStrength();

    GLuint GetTexture(TextureType type);
    Shader* GetShader(ShaderType type);
    Framebuffer* GetFramebuffer(FramebufferType type);
    Matrices* GetMatrices();

private:
    Renderer() {}

    static Renderer* instance;

    uint32_t skyboxSideSize = 256;
    uint32_t irradianceSideSize = 32;
    uint32_t prefilteredSideSize = 256;

    float exposure = 1.0;
    float ssaoStrength = 2.0;
    float ssaoRadius = 0.5;
    float bloomStrength = 0.3;
    float dofMinDistance = 1.0;
    float dofMaxDistance = 1.0;
    float dofFocusDistance = 1.0;
    float fogStart = 0.0;
    float fogEnd = 0.0;
    float fogHeight = 0.0;
    float fogIntensity = 1.0;
    float ssrRayStep = 0.01;
    float ssgiStrength = 1.5;

    int ssrMaxSteps = 100;
    int ssrMaxBinarySearchSteps = 10;
    int blurIterations = 8;
    int samples = 64;

    bool horizontal = true;
    bool buffer = true;
    bool ssrEnabled = false;
    bool ssgiEnabled = false;

    std::string shadersDir;

    Matrices m;

    std::vector<glm::vec3> ssaoSamples, noise;
    std::vector<Framebuffer*> pingPongBuffers, ssgiPingPong;

    std::shared_ptr<Framebuffer> capture, captureIrr, captureSpc, captureBRDF;

    std::unordered_map<TextureType, GLuint> textures;
    std::unordered_map<ShaderType, std::shared_ptr<Shader>> shaders;
    std::unordered_map<FramebufferType, std::shared_ptr<Framebuffer>> framebuffers;
};
