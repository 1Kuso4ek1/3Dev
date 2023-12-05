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
    delete instance;
}

void Renderer::Init(sf::Vector2u fbSize, const std::string& environmentMapFilename, uint32_t skyboxSideSize, uint32_t irradianceSideSize, uint32_t prefilteredSideSize, float bloomResolutionScale)
{
    shaders[ShaderType::Deferred] = std::make_shared<Shader>(shadersDir + "vertex.vs", shadersDir + "deferred.fs");
    shaders[ShaderType::Forward] = std::make_shared<Shader>(shadersDir + "vertex.vs", shadersDir + "forward.fs");
    shaders[ShaderType::LightingPass] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "fragment.fs");
    shaders[ShaderType::SSAO] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "ssao.fs");
    shaders[ShaderType::Skybox] = std::make_shared<Shader>(shadersDir + "skybox.vs", shadersDir + "skybox.fs");
    shaders[ShaderType::Depth] = std::make_shared<Shader>(shadersDir + "depth.vs", shadersDir + "depth.fs");
    shaders[ShaderType::Post] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "post.fs");
    shaders[ShaderType::Environment] = std::make_shared<Shader>(shadersDir + "environment.vs", shadersDir + "environment.fs");
    shaders[ShaderType::Irradiance] = std::make_shared<Shader>(shadersDir + "irradiance.vs", shadersDir + "irradiance.fs");
    shaders[ShaderType::Filtering] = std::make_shared<Shader>(shadersDir + "spcfiltering.vs", shadersDir + "spcfiltering.fs");
    shaders[ShaderType::BRDF] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "brdf.fs");
    shaders[ShaderType::Bloom] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "bloom.fs");

    framebuffers[FramebufferType::GBuffer] = std::make_shared<Framebuffer>(shaders[ShaderType::LightingPass].get(), fbSize.x, fbSize.y, false, true, 7);

    framebuffers[FramebufferType::Main] = std::make_shared<Framebuffer>(shaders[ShaderType::Post].get(), fbSize.x, fbSize.y, false, false);
    framebuffers[FramebufferType::Transparency] = std::make_shared<Framebuffer>(shaders[ShaderType::Post].get(), fbSize.x, fbSize.y);

    framebuffers[FramebufferType::BloomPingPong0] = std::make_shared<Framebuffer>(shaders[ShaderType::Bloom].get(), fbSize.x / bloomResolutionScale, fbSize.y / bloomResolutionScale, false, false, 1, GL_LINEAR);
    framebuffers[FramebufferType::BloomPingPong1] = std::make_shared<Framebuffer>(shaders[ShaderType::Bloom].get(), fbSize.x / bloomResolutionScale, fbSize.y / bloomResolutionScale, false, false, 1, GL_LINEAR);

    framebuffers[FramebufferType::SSAO] = std::make_shared<Framebuffer>(shaders[ShaderType::SSAO].get(), fbSize.x, fbSize.y, false, false, 1, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_R16F, GL_RED);
    
    std::random_device dev;
    std::default_random_engine eng(dev());
    std::uniform_real_distribution<float> dist(0.0, 1.0);

    ssaoSamples.reserve(64);
    noise.reserve(16);

    auto lerp = [](float a, float b, float f)
    {
        return a + f * (b - a);
    };

    for(int i = 0; i < 64; i++)
    {
        glm::vec3 sample(dist(eng) * 2.0 - 1.0, dist(eng) * 2.0 - 1.0, dist(eng));
        ssaoSamples[i] = glm::normalize(sample) * dist(eng) * lerp(0.1, 1.0, pow((float)i / 64.0, 2));
    }

    for(int i = 0; i < 16; i++)
        noise[i] = glm::vec3(dist(eng) * 2.0 - 1.0, dist(eng) * 2.0 - 1.0, 0.0f);

    textures[TextureType::Noise] = TextureManager::GetInstance()->CreateTexture(4, 4, false, GL_NEAREST, GL_REPEAT, GL_RGBA16F, GL_RGB, false, &noise[0]);

    shaders[ShaderType::SSAO]->Bind();
    /*shaders[ShaderType::SSAO]->Bind();
    shaders[ShaderType::SSAO]->SetUniform1i("gposition", 15);
    shaders[ShaderType::SSAO]->SetUniform1i("gnormal", 16);
    shaders[ShaderType::SSAO]->SetUniform1i("noise", 17);
    shaders[ShaderType::SSAO]->SetUniformMatrix4("projection", m.GetProjection());*/
    for(int i = 0; i < 64; i++)
        shaders[ShaderType::SSAO]->SetUniform3f("samples[" + std::to_string(i) + "]", ssaoSamples[i].x, ssaoSamples[i].y, ssaoSamples[i].z);

    capture = std::make_shared<Framebuffer>(nullptr, skyboxSideSize, skyboxSideSize, false, false);
    captureIrr = std::make_shared<Framebuffer>(nullptr, irradianceSideSize, irradianceSideSize, false, false);
    captureSpc = std::make_shared<Framebuffer>(nullptr, prefilteredSideSize, prefilteredSideSize, false, false);
    captureBRDF = std::make_shared<Framebuffer>(shaders[ShaderType::BRDF].get(), 128, 128, false, false);

    LoadEnvironment(environmentMapFilename);

    Log::Write("Renderer successfully initialized", Log::Type::Info);
}

void Renderer::LoadEnvironment(const std::string& environmentMapFilename)
{
    if(textures.find(TextureType::Skybox) != textures.end())
        TextureManager::GetInstance()->DeleteTexture("environment");
    
    GLuint environment = TextureManager::GetInstance()->LoadTexture(environmentMapFilename, "environment");
    
    GLuint cubemap = capture->CaptureCubemap(shaders[ShaderType::Environment].get(),
                                             environment, m, textures.find(TextureType::Skybox) == textures.end() ? 0 : textures[TextureType::Skybox]);
    textures[TextureType::Skybox] = cubemap;

    GLuint irr = captureIrr->CaptureCubemap(shaders[ShaderType::Irradiance].get(), cubemap, m, true,
                                            textures.find(TextureType::Irradiance) == textures.end() ? 0 : textures[TextureType::Irradiance]);
    textures[TextureType::Irradiance] = irr;

    GLuint filtered = captureSpc->CaptureCubemapMipmaps(shaders[ShaderType::Filtering].get(), cubemap, m, 8, 1024,
                                                        textures.find(TextureType::Prefiltered) == textures.end() ? 0 : textures[TextureType::Prefiltered]);
    textures[TextureType::Prefiltered] = filtered;

    if(textures.find(TextureType::LUT) == textures.end())
    {
        captureBRDF->Capture();
        textures[TextureType::LUT] = captureBRDF->GetTexture();
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
