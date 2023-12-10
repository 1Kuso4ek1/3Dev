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

void Renderer::Init(sf::Vector2u fbSize, const std::string& environmentMapFilename, uint32_t skyboxSideSize, uint32_t irradianceSideSize, uint32_t prefilteredSideSize, float bloomResolutionScale, bool useRGBA16F)
{
    shaders[ShaderType::Deferred] = std::make_shared<Shader>(shadersDir + "vertex.vs", shadersDir + "deferred.fs");
    shaders[ShaderType::Forward] = std::make_shared<Shader>(shadersDir + "vertex.vs", shadersDir + "forward.fs");
    shaders[ShaderType::LightingPass] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "fragment.fs");
    shaders[ShaderType::SSAO] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "ssao.fs");
    shaders[ShaderType::Decals] = std::make_shared<Shader>(shadersDir + "decals.vs", shadersDir + "decals.fs");
    shaders[ShaderType::Skybox] = std::make_shared<Shader>(shadersDir + "skybox.vs", shadersDir + "skybox.fs");
    shaders[ShaderType::Depth] = std::make_shared<Shader>(shadersDir + "depth.vs", shadersDir + "depth.fs");
    shaders[ShaderType::Post] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "post.fs");
    shaders[ShaderType::Environment] = std::make_shared<Shader>(shadersDir + "environment.vs", shadersDir + "environment.fs");
    shaders[ShaderType::Irradiance] = std::make_shared<Shader>(shadersDir + "irradiance.vs", shadersDir + "irradiance.fs");
    shaders[ShaderType::Filtering] = std::make_shared<Shader>(shadersDir + "spcfiltering.vs", shadersDir + "spcfiltering.fs");
    shaders[ShaderType::BRDF] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "brdf.fs");
    shaders[ShaderType::Bloom] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "bloom.fs");

    framebuffers[FramebufferType::GBuffer] = std::make_shared<Framebuffer>(shaders[ShaderType::LightingPass].get(), fbSize.x, fbSize.y, false, true, 7, GL_LINEAR, GL_CLAMP_TO_EDGE, useRGBA16F ? GL_RGBA16F : GL_RGBA32F, GL_RGBA);
    framebuffers[FramebufferType::DecalsGBuffer] = std::make_shared<Framebuffer>(nullptr, fbSize.x, fbSize.y, false, false, 4, GL_LINEAR, GL_CLAMP_TO_EDGE, useRGBA16F ? GL_RGBA16F : GL_RGBA32F, GL_RGBA);

    framebuffers[FramebufferType::Main] = std::make_shared<Framebuffer>(shaders[ShaderType::Post].get(), fbSize.x, fbSize.y, false, false);
    framebuffers[FramebufferType::Transparency] = std::make_shared<Framebuffer>(shaders[ShaderType::Post].get(), fbSize.x, fbSize.y);

    framebuffers[FramebufferType::BloomPingPong0] = std::make_shared<Framebuffer>(shaders[ShaderType::Bloom].get(), fbSize.x / bloomResolutionScale, fbSize.y / bloomResolutionScale, false, false, 1, GL_LINEAR);
    framebuffers[FramebufferType::BloomPingPong1] = std::make_shared<Framebuffer>(shaders[ShaderType::Bloom].get(), fbSize.x / bloomResolutionScale, fbSize.y / bloomResolutionScale, false, false, 1, GL_LINEAR);

    framebuffers[FramebufferType::SSAO] = std::make_shared<Framebuffer>(shaders[ShaderType::SSAO].get(), fbSize.x / 2.0, fbSize.y / 2.0, false, false, 1, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_R16F, GL_RED);
    
    std::random_device dev;
    std::default_random_engine eng(dev());
    std::uniform_real_distribution<float> dist(0.0, 1.0);

    ssaoSamples.reserve(samples);
    noise.reserve(4096);

    auto lerp = [](float a, float b, float f)
    {
        return a + f * (b - a);
    };

    for(int i = 0; i < samples; i++)
    {
        glm::vec3 sample(dist(eng) * 2.0 - 1.0, dist(eng) * 2.0 - 1.0, dist(eng));
        ssaoSamples[i] = glm::normalize(sample) * dist(eng) * lerp(0.1, 1.0, pow((float)i / samples, 2));
    }

    for(int i = 0; i < 4096; i++)
        noise[i] = glm::vec3(dist(eng) * 2.0 - 1.0, dist(eng) * 2.0 - 1.0, 0.0f);

    textures[TextureType::Noise] = TextureManager::GetInstance()->CreateTexture(64, 64, false, GL_NEAREST, GL_REPEAT, GL_RGB16F, GL_RGB, false, &noise[0]);

    shaders[ShaderType::SSAO]->Bind();
    for(int i = 0; i < samples; i++)
        shaders[ShaderType::SSAO]->SetUniform3f("samples[" + std::to_string(i) + "]", ssaoSamples[i].x, ssaoSamples[i].y, ssaoSamples[i].z);

    capture = std::make_shared<Framebuffer>(nullptr, skyboxSideSize, skyboxSideSize, false, false);
    captureIrr = std::make_shared<Framebuffer>(nullptr, irradianceSideSize, irradianceSideSize, false, false);
    captureSpc = std::make_shared<Framebuffer>(nullptr, prefilteredSideSize, prefilteredSideSize, false, false);
    captureBRDF = std::make_shared<Framebuffer>(shaders[ShaderType::BRDF].get(), 128, 128, false, false);

    pingPongBuffers = 
    {
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong0),
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong1)
    };

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

void Renderer::SetExposure(float exposure)
{
    this->exposure = exposure;
}

void Renderer::SetSSAOStrength(float strength)
{
    ssaoStrength = strength;
}

void Renderer::SetSSAORadius(float radius)
{
    ssaoRadius = radius;
}

void Renderer::SetSSAOSamples(int samples)
{
    this->samples = samples > 128 ? 128 : samples;
}

void Renderer::SetBloomStrength(float strength)
{
    bloomStrength = strength;
}

void Renderer::SetBlurIterations(int iterations)
{
    blurIterations = iterations;
}

void Renderer::Bloom()
{
    horizontal = buffer = true;

    if(bloomStrength > 0)
    {
        pingPongBuffers[0]->Bind();
        glViewport(0, 0, pingPongBuffers[0]->GetSize().x, pingPongBuffers[0]->GetSize().y);
        shaders[ShaderType::Post]->Bind();
        shaders[ShaderType::Post]->SetUniform1i("transparentBuffer", false);
        shaders[ShaderType::Post]->SetUniform1i("rawColor", true);
        framebuffers[FramebufferType::Main]->Draw();
        shaders[ShaderType::Post]->Bind();
        shaders[ShaderType::Post]->SetUniform1i("transparentBuffer", true);
        shaders[ShaderType::Post]->SetUniform1i("rawColor", true);
        framebuffers[FramebufferType::Transparency]->Draw();

        for(int i = 0; i < blurIterations; i++)
        {
            pingPongBuffers[buffer]->Bind();
            Renderer::GetInstance()->GetShader(Renderer::ShaderType::Bloom)->Bind();
            Renderer::GetInstance()->GetShader(Renderer::ShaderType::Bloom)->SetUniform1i("horizontal", horizontal);
            pingPongBuffers[!buffer]->Draw();
            buffer = !buffer; horizontal = !horizontal;
        }

        Framebuffer::Unbind();
    }
}

void Renderer::SSAO()
{
    framebuffers[FramebufferType::SSAO]->Bind();
    glViewport(0, 0, framebuffers[FramebufferType::SSAO]->GetSize().x, framebuffers[FramebufferType::SSAO]->GetSize().y);
    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 5));
    glActiveTexture(GL_TEXTURE16);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 6));
    glActiveTexture(GL_TEXTURE17);
    glBindTexture(GL_TEXTURE_2D, textures[Renderer::TextureType::Noise]);
    shaders[ShaderType::SSAO]->Bind();
    shaders[ShaderType::SSAO]->SetUniform1i("gposition", 15);
    shaders[ShaderType::SSAO]->SetUniform1i("gnormal", 16);
    shaders[ShaderType::SSAO]->SetUniform1i("noise", 17);
    shaders[ShaderType::SSAO]->SetUniform1i("numSamples", samples);
    shaders[ShaderType::SSAO]->SetUniform1f("radius", ssaoRadius);
    shaders[ShaderType::SSAO]->SetUniform1f("strength", ssaoStrength);
    shaders[ShaderType::SSAO]->SetUniformMatrix4("projection", m.GetProjection());
    framebuffers[FramebufferType::SSAO]->Draw();
    Framebuffer::Unbind();
}

void Renderer::DrawFramebuffers()
{
    auto size = framebuffers[FramebufferType::Main]->GetSize();
    glViewport(0, 0, size.x, size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, pingPongBuffers[buffer]->GetTexture());
    shaders[ShaderType::Post]->Bind();
    shaders[ShaderType::Post]->SetUniform1f("exposure", exposure);
    shaders[ShaderType::Post]->SetUniform1f("bloomStrength", bloomStrength);
    shaders[ShaderType::Post]->SetUniform1i("bloom", 15);
    shaders[ShaderType::Post]->SetUniform1i("rawColor", false);
    shaders[ShaderType::Post]->SetUniform1i("transparentBuffer", false);
    framebuffers[FramebufferType::Main]->Draw();
    glDisable(GL_DEPTH_TEST);
    shaders[ShaderType::Post]->Bind();
    shaders[ShaderType::Post]->SetUniform1i("transparentBuffer", true);
    framebuffers[FramebufferType::Transparency]->Draw();
    glEnable(GL_DEPTH_TEST);
}

float Renderer::GetExposure()
{
    return exposure;
}

float Renderer::GetSSAOStrength()
{
    return ssaoStrength;
}

float Renderer::GetSSAORadius()
{
    return ssaoRadius;
}

int Renderer::GetSSAOSamples()
{
    return samples;
}

float Renderer::GetBloomStrength()
{
    return bloomStrength;
}

int Renderer::GetBlurIterations()
{
    return blurIterations;
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
