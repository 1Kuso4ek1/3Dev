#include <Renderer.hpp>

Renderer* Renderer::instance = nullptr;

Renderer* Renderer::GetInstance()
{
    if(instance) return instance;
    else
    {
        instance = new Renderer;
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
    shaders[ShaderType::Main] = std::make_shared<Shader>(std::string(SHADERS_DIRECTORY) + "vertex.vs", std::string(SHADERS_DIRECTORY) + "fragment.frag");
    shaders[ShaderType::Skybox] = std::make_shared<Shader>(std::string(SHADERS_DIRECTORY) + "skybox.vs", std::string(SHADERS_DIRECTORY) + "skybox.frag");
    shaders[ShaderType::Depth] = std::make_shared<Shader>(std::string(SHADERS_DIRECTORY) + "depth.vs", std::string(SHADERS_DIRECTORY) + "depth.frag");
    shaders[ShaderType::Post] = std::make_shared<Shader>(std::string(SHADERS_DIRECTORY) + "post.vs", std::string(SHADERS_DIRECTORY) + "post.frag");
    shaders[ShaderType::Environment] = std::make_shared<Shader>(std::string(SHADERS_DIRECTORY) + "environment.vs", std::string(SHADERS_DIRECTORY) + "environment.frag");
    shaders[ShaderType::Irradiance] = std::make_shared<Shader>(std::string(SHADERS_DIRECTORY) + "irradiance.vs", std::string(SHADERS_DIRECTORY) + "irradiance.frag");
    shaders[ShaderType::Filtering] = std::make_shared<Shader>(std::string(SHADERS_DIRECTORY) + "spcfiltering.vs", std::string(SHADERS_DIRECTORY) + "spcfiltering.frag");
    shaders[ShaderType::BRDF] = std::make_shared<Shader>(std::string(SHADERS_DIRECTORY) + "brdf.vs", std::string(SHADERS_DIRECTORY) + "brdf.frag");

    framebuffers[FramebufferType::Main] = std::make_shared<Framebuffer>(shaders[ShaderType::Post].get(), fbSize.x, fbSize.y);
    framebuffers[FramebufferType::Transparency] = std::make_shared<Framebuffer>(shaders[ShaderType::Post].get(), fbSize.x, fbSize.y);

    Framebuffer capture(nullptr, skyboxSideSize, skyboxSideSize), captureIrr(nullptr, irradianceSideSize, irradianceSideSize),
                captureSpc(nullptr, prefilteredSideSize, prefilteredSideSize), captureBRDF(shaders[ShaderType::BRDF].get(), 512, 512);

    GLuint cubemap = capture.CaptureCubemap(shaders[ShaderType::Environment].get(), LoadHDRTexture(environmentMapFilename), m);
    textures[TextureType::Skybox] = cubemap;

    GLuint irr = captureIrr.CaptureCubemap(shaders[ShaderType::Irradiance].get(), cubemap, m, true);
    textures[TextureType::Irradiance] = irr;

    GLuint filtered = captureSpc.CaptureCubemapMipmaps(shaders[ShaderType::Filtering].get(), cubemap, m, 8, 1024);
    textures[TextureType::Prefiltered] = filtered;

    captureBRDF.Capture(0);
    GLuint BRDF = captureBRDF.GetTexture();
    textures[TextureType::LUT] = BRDF;

    Log::Write("Renderer successfully initialized", Log::Type::Info);
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
