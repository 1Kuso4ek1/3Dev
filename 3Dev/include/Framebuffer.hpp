#pragma once
#include "Utils.hpp"
#include "Shader.hpp"
#include "TextureManager.hpp"
#include "Mesh.hpp"
#include "Matrices.hpp"

/*
 * This class is used to create a framebuffer and provides basic functionality for writing, reading, and rendering buffer data.
 */
class Framebuffer
{
public:
    /*
     * Main constructor
     * @param shader pointer to the shader for rendering buffer texture
     * @param w width of the framebuffer texture
     * @param h height of the framebuffer texture
     */
    Framebuffer(Shader* shader, int w, int h, bool isDepth = false, bool createDepthAttachment = true, uint32_t attachments = 1,
                GLint filter = GL_NEAREST, GLint wrap = GL_CLAMP_TO_EDGE, GLuint internalFormat = GL_RGBA16F, GLuint format = GL_RGBA);
    ~Framebuffer();

    void Resize(int w, int h);

    /*
     * Calculates pixel size of current texture (useful in shaders)
     * @param v width and height of the current texture
     */
    void CalcPixelSize(glm::vec2 v);

    // Used to bind framebuffer
    void Bind();

    // Used to unbind framebuffer
    static void Unbind();

    // Draw the buffer texture on the screen
    void Draw(uint32_t attachment = 0);

    GLuint Capture(GLuint texture = 0, GLuint output = 0);
    GLuint CaptureCubemap(Shader* shader, GLuint tex, Matrices& m, bool isSkybox = false, GLuint output = 0);
    GLuint CaptureCubemapMipmaps(Shader* shader, GLuint tex, Matrices& m, int maxLevel, int samples, GLuint output = 0);

    static float* GetPixels(glm::ivec2 coords, glm::ivec2 size);

    /*
     * Returns the buffer texture
     * @param depth return depth texture (if true) or color texture (if false)
     */
    GLuint GetTexture(bool depth = false, uint32_t attachment = 0);

    glm::ivec2 GetSize();

private:
    Shader* shader;

    glm::vec2 pixelsize;
    glm::ivec2 size;

    std::vector<glm::mat4> views =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

    std::unordered_map<int, std::string> errorStatus = 
    {
        { 36054, "Incomplete attachment" },
        { 36057, "Incomplete dimensions" },
        { 36055, "Missing attachment" },
        { 36061, "Unsupported" }
    };

    std::vector<GLuint> textures;
    std::shared_ptr<Mesh> rectangle;
    GLuint fbo, depth = 0;
};
