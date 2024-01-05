#pragma once
#include "Utils.hpp"
#include "Multithreading.hpp"
#include <stb_image.h>

class TextureManager
{
public:
    static TextureManager* GetInstance();
    static void DeleteInstance();

    GLuint CreateTexture(uint32_t w, uint32_t h, bool depth = false, GLint filter = GL_NEAREST, GLint wrap = GL_CLAMP_TO_EDGE, GLuint internalFormat = GL_RGBA16F,
                         GLuint format = GL_RGBA, bool generateMipmap = false, void* data = nullptr, std::string name = "texture");
    GLuint LoadTexture(std::string filename, std::string name = "texture");
    GLuint CreateCubemap(uint32_t size, GLuint filter = GL_LINEAR, std::string name = "cubemap");

    void ResizeTexture(std::string name, bool depth, uint32_t w, uint32_t h);
    void DeleteTexture(std::string name);
    void MakeFilenamesRelativeTo(std::string base);

    GLuint GetTexture(std::string name);
    std::string GetName(GLuint id);
    std::string GetFilename(std::string name);
    std::string GetFilename(GLuint id);

private:
    TextureManager() {}

    static TextureManager* instance;

    std::unordered_map<std::string, GLuint> textures;
    std::unordered_map<std::string, std::string> filenames;
};
