#include <Renderer.hpp>

Renderer& Renderer::GetInstance()
{
    static std::unique_ptr<Renderer> instance(new Renderer());
    return *instance;
}

void Renderer::Init(sf::Vector2u fbSize, const std::string& environmentMapFilename, uint32_t skyboxSideSize, uint32_t irradianceSideSize, uint32_t prefilteredSideSize, float bloomResolutionScale, bool useRGBA16F)
{
    shaders[ShaderType::Deferred] = std::make_shared<Shader>(shadersDir + "vertex.vs", shadersDir + "deferred.fs");
    shaders[ShaderType::Forward] = std::make_shared<Shader>(shadersDir + "vertex.vs", shadersDir + "forward.fs");
    shaders[ShaderType::LightingPass] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "fragment.fs");
    shaders[ShaderType::SSAO] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "ssao.fs");
    shaders[ShaderType::SSGI] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "ssgi.fs");
    shaders[ShaderType::SSR] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "ssr.fs");
    shaders[ShaderType::Fog] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "fog.fs");
    shaders[ShaderType::Decals] = std::make_shared<Shader>(shadersDir + "decals.vs", shadersDir + "decals.fs");
    shaders[ShaderType::Skybox] = std::make_shared<Shader>(shadersDir + "skybox.vs", shadersDir + "skybox.fs");
    shaders[ShaderType::Depth] = std::make_shared<Shader>(shadersDir + "depth.vs", shadersDir + "depth.fs");
    shaders[ShaderType::Post] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "post.fs");
    shaders[ShaderType::Environment] = std::make_shared<Shader>(shadersDir + "environment.vs", shadersDir + "environment.fs");
    shaders[ShaderType::Irradiance] = std::make_shared<Shader>(shadersDir + "irradiance.vs", shadersDir + "irradiance.fs");
    shaders[ShaderType::Filtering] = std::make_shared<Shader>(shadersDir + "spcfiltering.vs", shadersDir + "spcfiltering.fs");
    shaders[ShaderType::BRDF] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "brdf.fs");
    shaders[ShaderType::Bloom] = std::make_shared<Shader>(shadersDir + "post.vs", shadersDir + "bloom.fs");

    framebuffers[FramebufferType::GBuffer] = std::make_shared<Framebuffer>(shaders[ShaderType::LightingPass].get(), fbSize.x, fbSize.y, false, true, 5, GL_LINEAR, GL_CLAMP_TO_EDGE, useRGBA16F ? GL_RGBA16F : GL_RGBA32F, GL_RGBA);
    framebuffers[FramebufferType::DecalsGBuffer] = std::make_shared<Framebuffer>(nullptr, fbSize.x, fbSize.y, false, false, 4, GL_LINEAR, GL_CLAMP_TO_EDGE, useRGBA16F ? GL_RGBA16F : GL_RGBA32F, GL_RGBA);

    framebuffers[FramebufferType::Main] = std::make_shared<Framebuffer>(shaders[ShaderType::Post].get(), fbSize.x, fbSize.y, false, false);
    framebuffers[FramebufferType::Transparency] = std::make_shared<Framebuffer>(shaders[ShaderType::Post].get(), fbSize.x, fbSize.y);

    framebuffers[FramebufferType::BloomPingPong0] = std::make_shared<Framebuffer>(shaders[ShaderType::Bloom].get(), fbSize.x / bloomResolutionScale, fbSize.y / bloomResolutionScale, false, false, 1, GL_LINEAR);
    framebuffers[FramebufferType::BloomPingPong1] = std::make_shared<Framebuffer>(shaders[ShaderType::Bloom].get(), fbSize.x / bloomResolutionScale, fbSize.y / bloomResolutionScale, false, false, 1, GL_LINEAR);

    framebuffers[FramebufferType::SSGIPingPong0] = std::make_shared<Framebuffer>(shaders[ShaderType::Bloom].get(), fbSize.x / 8.0, fbSize.y / 8.0, false, false, 1, GL_LINEAR);
    framebuffers[FramebufferType::SSGIPingPong1] = std::make_shared<Framebuffer>(shaders[ShaderType::Bloom].get(), fbSize.x / 8.0, fbSize.y / 8.0, false, false, 1, GL_LINEAR);

    framebuffers[FramebufferType::SSAO] = std::make_shared<Framebuffer>(shaders[ShaderType::SSAO].get(), fbSize.x / 2.0, fbSize.y / 2.0, false, false, 1, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_R16F, GL_RED);

    framebuffers[FramebufferType::SSGI] = std::make_shared<Framebuffer>(shaders[ShaderType::SSGI].get(), fbSize.x / 2.0, fbSize.y / 2.0, false, false, 1, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_RGBA16F, GL_RGBA);

    framebuffers[FramebufferType::SSR] = std::make_shared<Framebuffer>(shaders[ShaderType::SSR].get(), fbSize.x, fbSize.y, false, false, 1, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_RGB16F, GL_RGB);

    framebuffers[FramebufferType::Fog] = std::make_shared<Framebuffer>(shaders[ShaderType::Fog].get(), fbSize.x, fbSize.y, false, false, 1, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_RGBA16F, GL_RGBA);
    
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

    textures[TextureType::Noise] = TextureManager::GetInstance().CreateTexture(64, 64, false, GL_NEAREST, GL_REPEAT, GL_RGB16F, GL_RGB, false, &noise[0]);

    shaders[ShaderType::SSAO]->Bind();
    for(int i = 0; i < samples; i++)
        shaders[ShaderType::SSAO]->SetUniform3f("samples[" + std::to_string(i) + "]", ssaoSamples[i].x, ssaoSamples[i].y, ssaoSamples[i].z);

    this->skyboxSideSize = skyboxSideSize;
    this->irradianceSideSize = irradianceSideSize;
    this->prefilteredSideSize = prefilteredSideSize;

    capture = std::make_shared<Framebuffer>(nullptr, skyboxSideSize, skyboxSideSize, false, false);
    captureIrr = std::make_shared<Framebuffer>(nullptr, irradianceSideSize, irradianceSideSize, false, false);
    captureSpc = std::make_shared<Framebuffer>(nullptr, prefilteredSideSize, prefilteredSideSize, false, false);
    captureBRDF = std::make_shared<Framebuffer>(shaders[ShaderType::BRDF].get(), 128, 128, false, false);

    pingPongBuffers = 
    {
        Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::BloomPingPong0),
        Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::BloomPingPong1)
    };

    ssgiPingPong = 
    {
        Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::SSGIPingPong0),
        Renderer::GetInstance().GetFramebuffer(Renderer::FramebufferType::SSGIPingPong1)
    };

    LoadEnvironment(environmentMapFilename);

    Log::Write("Renderer successfully initialized", Log::Type::Info);
}

void Renderer::LoadEnvironment(const std::string& environmentMapFilename)
{
    if(capture->GetSize().x != skyboxSideSize)
        capture->Resize(skyboxSideSize, skyboxSideSize);
    if(captureIrr->GetSize().x != irradianceSideSize)
        captureIrr->Resize(irradianceSideSize, irradianceSideSize);
    if(captureSpc->GetSize().x != prefilteredSideSize)
        captureSpc->Resize(prefilteredSideSize, prefilteredSideSize);

    if(textures.find(TextureType::Skybox) != textures.end())
        TextureManager::GetInstance().DeleteTexture("environment");
    
    GLuint environment = TextureManager::GetInstance().LoadTexture(environmentMapFilename, "environment");
    
    GLuint cubemap = capture->CaptureCubemap(shaders[ShaderType::Environment].get(),
                                             environment, m, textures.find(TextureType::Skybox) == textures.end() ? 0 : textures[TextureType::Skybox]);
    textures[TextureType::Skybox] = cubemap;

    GLuint irr = captureIrr->CaptureCubemap(shaders[ShaderType::Irradiance].get(), cubemap, m, true, 0);
    textures[TextureType::Irradiance] = irr;

    GLuint filtered = captureSpc->CaptureCubemapMipmaps(shaders[ShaderType::Filtering].get(), cubemap, m, 8, 1024, 0);
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

void Renderer::SetSkyboxResolution(uint32_t res)
{
    this->skyboxSideSize = res;
}

void Renderer::SetIrradianceResolution(uint32_t res)
{
    this->irradianceSideSize = res;
}

void Renderer::SetPrefilteredResolution(uint32_t res)
{
    this->prefilteredSideSize = res;
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

void Renderer::SetDOFMinDistance(float dist)
{
    dofMinDistance = dist;
}

void Renderer::SetDOFMaxDistance(float dist)
{
    dofMaxDistance = dist;
}

void Renderer::SetDOFFocusDistance(float dist)
{
    dofFocusDistance = dist;
}

void Renderer::SetBloomStrength(float strength)
{
    bloomStrength = strength;
}

void Renderer::SetBlurIterations(int iterations)
{
    blurIterations = iterations;
}

void Renderer::SetFogStart(float start)
{
    fogStart = start;
}

void Renderer::SetFogEnd(float end)
{
    fogEnd = end;
}

void Renderer::SetFogHeight(float height)
{
    fogHeight = height;
}

void Renderer::SetFogSkyHeight(float skyHeight)
{
    fogSkyHeight = skyHeight;
}

void Renderer::SetFogIntensity(float intensity)
{
    fogIntensity = intensity;
}

void Renderer::SetIsSSREnabled(bool ssrEnabled)
{
    this->ssrEnabled = ssrEnabled;
}

void Renderer::SetSSRRayStep(float step)
{
    ssrRayStep = step;
}

void Renderer::SetSSRMaxSteps(int steps)
{
    ssrMaxSteps = steps;
}

void Renderer::SetSSRMaxBinarySearchSteps(int steps)
{
    ssrMaxBinarySearchSteps = steps;
}

void Renderer::SetIsSSGIEnabled(bool ssgiEnabled)
{
    this->ssgiEnabled = ssgiEnabled;
}

void Renderer::SetSSGIStrength(float ssgiStrength)
{
    this->ssgiStrength = ssgiStrength;
}

void Renderer::Bloom()
{
    if(bloomStrength <= 0) return;

    horizontal = buffer = true;

    glActiveTexture(GL_TEXTURE16);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::SSR]->GetTexture());

    pingPongBuffers[0]->Bind();
    glViewport(0, 0, pingPongBuffers[0]->GetSize().x, pingPongBuffers[0]->GetSize().y);
    shaders[ShaderType::Post]->Bind();
    shaders[ShaderType::Post]->SetUniform1i("transparentBuffer", false);
    shaders[ShaderType::Post]->SetUniform1i("ssrEnabled", ssrEnabled);
    shaders[ShaderType::Post]->SetUniform1i("ssr", 16);
    shaders[ShaderType::Post]->SetUniform1i("rawColor", true);
    framebuffers[FramebufferType::Main]->Draw();
    shaders[ShaderType::Post]->Bind();
    shaders[ShaderType::Post]->SetUniform1i("transparentBuffer", true);
    shaders[ShaderType::Post]->SetUniform1i("rawColor", true);
    framebuffers[FramebufferType::Transparency]->Draw();

    glBindTexture(GL_TEXTURE_2D, pingPongBuffers[0]->GetTexture());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    for(int i = 0; i < blurIterations; i++)
    {
        pingPongBuffers[buffer]->Bind();
        Renderer::GetInstance().GetShader(Renderer::ShaderType::Bloom)->Bind();
        Renderer::GetInstance().GetShader(Renderer::ShaderType::Bloom)->SetUniform1f("lod", (i * 2.0) / float(blurIterations));
        Renderer::GetInstance().GetShader(Renderer::ShaderType::Bloom)->SetUniform1i("horizontal", horizontal);
        pingPongBuffers[!buffer]->Draw();

        glBindTexture(GL_TEXTURE_2D, pingPongBuffers[buffer]->GetTexture());
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        buffer = !buffer; horizontal = !horizontal;
    }

    Framebuffer::Unbind();
}

void Renderer::SSAO()
{
    if(ssaoStrength <= 0) return;

    framebuffers[FramebufferType::SSAO]->Bind();
    glViewport(0, 0, framebuffers[FramebufferType::SSAO]->GetSize().x, framebuffers[FramebufferType::SSAO]->GetSize().y);
    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 0));
    glActiveTexture(GL_TEXTURE16);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 2));
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

void Renderer::SSGI()
{
    if(!ssgiEnabled) return;

    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 0));
    glActiveTexture(GL_TEXTURE16);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::Main]->GetTexture(false));
    shaders[ShaderType::SSGI]->Bind();
    shaders[ShaderType::SSGI]->SetUniform1i("gposition", 15);
    shaders[ShaderType::SSGI]->SetUniform1i("galbedo", 16);
    shaders[ShaderType::SSGI]->SetUniform1f("strength", ssgiStrength);

    horizontal = buffer = true;

    ssgiPingPong[0]->Bind();
    glViewport(0, 0, ssgiPingPong[0]->GetSize().x, ssgiPingPong[0]->GetSize().y);
    framebuffers[FramebufferType::SSGI]->Draw();

    glBindTexture(GL_TEXTURE_2D, ssgiPingPong[0]->GetTexture());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    for(int i = 0; i < 32; i++)
    {
        ssgiPingPong[buffer]->Bind();
        Renderer::GetInstance().GetShader(Renderer::ShaderType::Bloom)->Bind();
        Renderer::GetInstance().GetShader(Renderer::ShaderType::Bloom)->SetUniform1f("lod", 1.0);
        Renderer::GetInstance().GetShader(Renderer::ShaderType::Bloom)->SetUniform1i("horizontal", horizontal);
        ssgiPingPong[!buffer]->Draw();

        glBindTexture(GL_TEXTURE_2D, ssgiPingPong[buffer]->GetTexture());
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        buffer = !buffer; horizontal = !horizontal;
    }

    Framebuffer::Unbind();
}

void Renderer::SSR()
{
    if(!ssrEnabled) return;

    framebuffers[FramebufferType::SSR]->Bind();
    glViewport(0, 0, framebuffers[FramebufferType::SSR]->GetSize().x, framebuffers[FramebufferType::SSR]->GetSize().y);
    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 0));
    glActiveTexture(GL_TEXTURE16);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 2));
    glActiveTexture(GL_TEXTURE17);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::Main]->GetTexture(false));
    glActiveTexture(GL_TEXTURE18);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 4));
    glActiveTexture(GL_TEXTURE19);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::DecalsGBuffer]->GetTexture(false, 1));
    glActiveTexture(GL_TEXTURE20);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::DecalsGBuffer]->GetTexture(false, 3));
    shaders[ShaderType::SSR]->Bind();
    shaders[ShaderType::SSR]->SetUniform1i("gposition", 15);
    shaders[ShaderType::SSR]->SetUniform1i("gnormal", 16);
    shaders[ShaderType::SSR]->SetUniform1i("gcolor", 17);
    shaders[ShaderType::SSR]->SetUniform1i("gcombined", 18);
    shaders[ShaderType::SSR]->SetUniform1i("decalsNormal", 19);
    shaders[ShaderType::SSR]->SetUniform1i("decalsCombined", 20);
    shaders[ShaderType::SSR]->SetUniform1f("rayStep", ssrRayStep);
    shaders[ShaderType::SSR]->SetUniform1i("maxSteps", ssrMaxSteps);
    shaders[ShaderType::SSR]->SetUniform1i("maxBinarySearchSteps", ssrMaxBinarySearchSteps);
    shaders[ShaderType::SSR]->SetUniformMatrix4("projection", m.GetProjection());
    shaders[ShaderType::SSR]->SetUniformMatrix4("view", m.GetView());
    framebuffers[FramebufferType::SSR]->Draw();
    Framebuffer::Unbind();
}

void Renderer::Fog(rp3d::Vector3 camPos)
{
    if(fogEnd == 0.0) return;

    framebuffers[FramebufferType::Fog]->Bind();
    glViewport(0, 0, framebuffers[FramebufferType::Fog]->GetSize().x, framebuffers[FramebufferType::Fog]->GetSize().y);
    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 0));
    glActiveTexture(GL_TEXTURE16);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 3));
    glActiveTexture(GL_TEXTURE17);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textures[TextureType::Irradiance]);
    shaders[ShaderType::Fog]->Bind();
    shaders[ShaderType::Fog]->SetUniform1i("gposition", 15);
    shaders[ShaderType::Fog]->SetUniform1i("gemission", 16);
    shaders[ShaderType::Fog]->SetUniform1i("irradiance", 17);
    shaders[ShaderType::Fog]->SetUniform1f("fogStart", fogStart);
    shaders[ShaderType::Fog]->SetUniform1f("fogEnd", fogEnd);
    shaders[ShaderType::Fog]->SetUniform1f("fogHeight", fogHeight);
    shaders[ShaderType::Fog]->SetUniform1f("fogSkyHeight", fogSkyHeight);
    shaders[ShaderType::Fog]->SetUniform3f("campos", camPos.x, camPos.y, camPos.z);
    shaders[ShaderType::Fog]->SetUniformMatrix4("invView", m.GetInverseView());
    framebuffers[FramebufferType::Fog]->Draw();
    Framebuffer::Unbind();
}

void Renderer::DrawFramebuffers()
{
    auto size = framebuffers[FramebufferType::Main]->GetSize();
    glViewport(0, 0, size.x, size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, pingPongBuffers[buffer]->GetTexture());
    glActiveTexture(GL_TEXTURE16);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::SSR]->GetTexture());

    if(ssrEnabled)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    glActiveTexture(GL_TEXTURE17);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 1));
    glActiveTexture(GL_TEXTURE18);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::GBuffer]->GetTexture(false, 4));
    glActiveTexture(GL_TEXTURE19);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::DecalsGBuffer]->GetTexture(false, 3));
    glActiveTexture(GL_TEXTURE20);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::DecalsGBuffer]->GetTexture(false, 0));
    glActiveTexture(GL_TEXTURE21);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::SSGIPingPong1]->GetTexture());
    glActiveTexture(GL_TEXTURE22);
    glBindTexture(GL_TEXTURE_2D, framebuffers[FramebufferType::Fog]->GetTexture());

    shaders[ShaderType::Post]->Bind();
    shaders[ShaderType::Post]->SetUniform1f("exposure", exposure);
    shaders[ShaderType::Post]->SetUniform1f("bloomStrength", bloomStrength);
    shaders[ShaderType::Post]->SetUniform1f("dofMinDistance", dofMinDistance);
    shaders[ShaderType::Post]->SetUniform1f("dofMaxDistance", dofMaxDistance);
    shaders[ShaderType::Post]->SetUniform1f("dofFocusDistance", dofFocusDistance);
    shaders[ShaderType::Post]->SetUniform1f("fogIntensity", fogIntensity);
    shaders[ShaderType::Post]->SetUniform1i("bloom", 15);
    shaders[ShaderType::Post]->SetUniform1i("ssr", 16);
    shaders[ShaderType::Post]->SetUniform1i("galbedo", 17);
    shaders[ShaderType::Post]->SetUniform1i("gcombined", 18);
    shaders[ShaderType::Post]->SetUniform1i("decalsCombined", 19);
    shaders[ShaderType::Post]->SetUniform1i("decalsAlbedo", 20);
    shaders[ShaderType::Post]->SetUniform1i("ssgi", 21);
    shaders[ShaderType::Post]->SetUniform1i("fog", 22);
    shaders[ShaderType::Post]->SetUniform1i("rawColor", false);
    shaders[ShaderType::Post]->SetUniform1i("fogEnabled", fogEnd != 0.0);
    shaders[ShaderType::Post]->SetUniform1i("ssrEnabled", ssrEnabled);
    shaders[ShaderType::Post]->SetUniform1i("ssgiEnabled", ssgiEnabled);
    shaders[ShaderType::Post]->SetUniform1i("transparentBuffer", false);
    framebuffers[FramebufferType::Main]->Draw();
    glDisable(GL_DEPTH_TEST);
    shaders[ShaderType::Post]->Bind();
    shaders[ShaderType::Post]->SetUniform1i("transparentBuffer", true);
    framebuffers[FramebufferType::Transparency]->Draw();
    glEnable(GL_DEPTH_TEST);
}

uint32_t Renderer::GetSkyboxResolution()
{
    return skyboxSideSize;
}

uint32_t Renderer::GetIrradianceResolution()
{
    return irradianceSideSize;
}

uint32_t Renderer::GetPrefilteredResolution()
{
    return prefilteredSideSize;
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

float Renderer::GetDOFMinDistance()
{
    return dofMinDistance;
}

float Renderer::GetDOFMaxDistance()
{
    return dofMaxDistance;
}

float Renderer::GetDOFFocusDistance()
{
    return dofFocusDistance;
}

float Renderer::GetFogStart()
{
    return fogStart;
}

float Renderer::GetFogEnd()
{
    return fogEnd;
}

float Renderer::GetFogHeight()
{
    return fogHeight;
}

float Renderer::GetFogSkyHeight()
{
    return fogSkyHeight;
}

float Renderer::GetFogIntensity()
{
    return fogIntensity;
}

float Renderer::GetBloomStrength()
{
    return bloomStrength;
}

int Renderer::GetBlurIterations()
{
    return blurIterations;
}

bool Renderer::IsSSREnabled()
{
    return ssrEnabled;
}

float Renderer::GetSSRRayStep()
{
    return ssrRayStep;
}

int Renderer::GetSSRMaxSteps()
{
    return ssrMaxSteps;
}

int Renderer::GetSSRMaxBinarySearchSteps()
{
    return ssrMaxBinarySearchSteps;
}

bool Renderer::IsSSGIEnabled()
{
    return ssgiEnabled;
}

float Renderer::GetSSGIStrength()
{
    return ssgiStrength;
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
