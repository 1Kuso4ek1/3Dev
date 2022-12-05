#include <Renderer.hpp>

Renderer* Renderer::instance = nullptr;

Renderer* Renderer::GetInstance()
{
    if(instance) return instance;
    else
    {
        instance = new Renderer;
        instance->SetShadersDirectory(SHADERS_DIRECTORY);
        return instance;
    }
}

void Renderer::DeleteInstance()
{
    if(instance)
        delete instance;
}

void Renderer::Init(sf::Vector2u fbSize, std::string environmentMapFilename, uint32_t skyboxSideSize, uint32_t irradianceSideSize, uint32_t prefilteredSideSize)
{
    shaders[ShaderType::Main] = std::make_shared<Shader>(shadersDir + "vertex.vs", shadersDir + "fragment.fs");
    shaders[ShaderType::Skybox] = std::make_shared<Shader>(shadersDir + "skybox.vs", shadersDir + "skybox.fs");
    shaders[ShaderType::Depth] = std::make_shared<Shader>(shadersDir + "depth.vs", shadersDir + "depth.fs");
    shaders[ShaderType::Post] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "post.fs");
    shaders[ShaderType::Environment] = std::make_shared<Shader>(shadersDir + "environment.vs", shadersDir + "environment.fs");
    shaders[ShaderType::Irradiance] = std::make_shared<Shader>(shadersDir + "irradiance.vs", shadersDir + "irradiance.fs");
    shaders[ShaderType::Filtering] = std::make_shared<Shader>(shadersDir + "spcfiltering.vs", shadersDir + "spcfiltering.fs");
    shaders[ShaderType::BRDF] = std::make_shared<Shader>(shadersDir + "brdf.vs", shadersDir + "brdf.fs");

    framebuffers[FramebufferType::Main] = std::make_shared<Framebuffer>(shaders[ShaderType::Post].get(), fbSize.x, fbSize.y);
    framebuffers[FramebufferType::Transparency] = std::make_shared<Framebuffer>(shaders[ShaderType::Post].get(), fbSize.x, fbSize.y);

    LoadEnvironment(environmentMapFilename, skyboxSideSize, irradianceSideSize, prefilteredSideSize);

    Log::Write("Renderer successfully initialized", Log::Type::Info);
}

void Renderer::LoadEnvironment(std::string environmentMapFilename, uint32_t skyboxSideSize, uint32_t irradianceSideSize, uint32_t prefilteredSideSize)
{
    for(auto& i : textures)
        if(i.second != 0 && i.first != TextureType::LUT)
            glDeleteTextures(1, &i.second);

    Framebuffer capture(nullptr, skyboxSideSize, skyboxSideSize), captureIrr(nullptr, irradianceSideSize, irradianceSideSize),
                captureSpc(nullptr, prefilteredSideSize, prefilteredSideSize), captureBRDF(shaders[ShaderType::BRDF].get(), 512, 512);

    GLuint cubemap = capture.CaptureCubemap(shaders[ShaderType::Environment].get(), TextureManager::GetInstance()->LoadTexture(environmentMapFilename), m);
    textures[TextureType::Skybox] = cubemap;

    GLuint irr = captureIrr.CaptureCubemap(shaders[ShaderType::Irradiance].get(), cubemap, m, true);
    textures[TextureType::Irradiance] = irr;

    GLuint filtered = captureSpc.CaptureCubemapMipmaps(shaders[ShaderType::Filtering].get(), cubemap, m, 8, 1024);
    textures[TextureType::Prefiltered] = filtered;

    if(textures.find(TextureType::LUT) == textures.end())
    {
        captureBRDF.Capture(0);
        GLuint BRDF = captureBRDF.GetTexture();
        textures[TextureType::LUT] = BRDF;
    }
}

void Renderer::SetShadersDirectory(std::string dir)
{
    shadersDir = dir;
}

GLuint Renderer::GetTexture(TextureType type)
{
    return textures[type];
}

Shader* Renderer::GetShader(ShaderType type)
{
    return shaders[type].get();
}

Framebuffer* Renderer::GetFramebuffer(FramebufferType type)
{
    return framebuffers[type].get();
}

Matrices* Renderer::GetMatrices()
{
    return &m;
}
