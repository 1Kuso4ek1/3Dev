#include <TextureManager.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

TextureManager& TextureManager::GetInstance()
{
    static std::unique_ptr<TextureManager> instance(new TextureManager());
    return *instance;
}

GLuint TextureManager::CreateTexture(uint32_t w, uint32_t h, bool depth, GLint filter, GLint wrap, GLuint internalFormat, GLuint format, bool generateMipmap, void* data, std::string name)
{
    GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, (depth ? GL_DEPTH_COMPONENT32F : internalFormat), w, h, 0, (depth ? GL_DEPTH_COMPONENT : format), GL_FLOAT, data);
    if(generateMipmap) glGenerateMipmap(GL_TEXTURE_2D);
    if(depth) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

    if(wrap == GL_CLAMP_TO_BORDER)
	{
		float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	}

    int nameCount = std::count_if(textures.begin(), textures.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });
    textures[name + (nameCount ? std::to_string(texture) : "")] = texture;

	return texture;
}

GLuint TextureManager::LoadTexture(std::string filename, std::string name)
{
    if(std::filesystem::path(filename).extension() == ".hdr")
    {
        int w, h, comp;
        float *data = stbi_loadf(filename.c_str(), &w, &h, &comp, 0);
        if (!data)
        {
            Log::Write("Can't load texture '" + filename + "'", Log::Type::Error);
            return 0;
        }

        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        stbi_image_free(data);

        int nameCount = std::count_if(textures.begin(), textures.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });
        textures[name + (nameCount ? std::to_string(texture) : "")] = texture;
        filenames[name + (nameCount ? std::to_string(texture) : "")] = filename;

        Log::Write("Texture " + filename + " loaded", Log::Type::Info);

        return texture;
    }
    else
    {
        GLuint texture = 0;
        glGenTextures(1, &texture);

        std::shared_ptr<sf::Image> image = std::make_shared<sf::Image>();

        auto loadImage = [image, filename]()
        {
            if(!image->loadFromFile(filename))
            {
                Log::Write("Can't load texture '" + filename + "'", Log::Type::Error);
                return;
            }
        };

        auto uploadToGPU = [image, texture, filename]()
        {
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->getSize().x, image->getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->getPixelsPtr());
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            Log::Write("Texture " + filename + " loaded", Log::Type::Info);
        };

        if(Multithreading::GetInstance().IsEnabled())
        {
            std::promise<void> promise;
            auto future = promise.get_future();
            
            std::thread th([image, filename, texture, loadImage, uploadToGPU](std::promise<void> pr)
            {
                loadImage();
                pr.set_value();
            }, std::move(promise));

            Multithreading::GetInstance().AddMainThreadJob(uploadToGPU);
            Multithreading::GetInstance().AddJob(th.get_id(), std::move(future));
            th.detach();
        }
        else
        {
            loadImage();
            uploadToGPU();
        }

        int nameCount = std::count_if(textures.begin(), textures.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });
        textures[name + (nameCount ? std::to_string(texture) : "")] = texture;
        filenames[name + (nameCount ? std::to_string(texture) : "")] = filename;

        return texture;
    }
}

GLuint TextureManager::CreateCubemap(uint32_t size, GLuint filter, std::string name)
{
    GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	for(int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    int nameCount = std::count_if(textures.begin(), textures.end(), [&](auto& p)
                    { return p.first.find(name) != std::string::npos; });
    textures[name + (nameCount ? std::to_string(texture) : "")] = texture;

	return texture;
}

void TextureManager::ResizeTexture(std::string name, bool depth, uint32_t w, uint32_t h)
{
    GLuint tex = textures[name];

    glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, (depth ? GL_DEPTH_COMPONENT32F : GL_RGBA16F), w, h, 0, (depth ? GL_DEPTH_COMPONENT : GL_RGBA), GL_FLOAT, NULL);
}

void TextureManager::DeleteTexture(std::string name)
{
    if(textures.find(name) == textures.end())
    {
        Log::Write("Can't delete unknown texture \"" + name + "\"", Log::Type::Error);
        return;
    }

    glDeleteTextures(1, &textures[name]);
    textures.erase(name);
}

void TextureManager::MakeFilenamesRelativeTo(std::string base)
{
    std::for_each(filenames.begin(), filenames.end(), [&](auto& a)
                  {
                      a.second = std::filesystem::relative(a.second, base).string();
                  });
}

GLuint TextureManager::GetTexture(std::string name)
{
    return textures[name];
}

std::string TextureManager::GetName(GLuint id)
{
    auto it = std::find_if(textures.begin(), textures.end(), [&](auto& a) { return a.second == id; });
    if(it == textures.end())
    {
        Log::Write("There's no texture with id " + std::to_string(id), Log::Type::Error);
        return "";
    }
    return it->first;
}

std::string TextureManager::GetFilename(std::string name)
{
    return filenames[name];
}

std::string TextureManager::GetFilename(GLuint id)
{
    return filenames[GetName(id)];
}
