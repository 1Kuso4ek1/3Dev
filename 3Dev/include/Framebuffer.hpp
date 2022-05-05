#pragma once
#include "3Dev.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
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
    Framebuffer(Shader* shader, int w, int h, bool isDepth = false, GLint filter = GL_NEAREST);
    ~Framebuffer();

    /*
     * Used to create a new framebuffer texture with another width and height
     * @param w width of the new framebuffer texture
     * @param h height of the new framebuffer texture
     */
    void RecreateTexture(int w, int h);
    
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
    void Draw();

    void Capture(GLuint texture);

    /*
     * Returns the buffer texture
     * @param depth return depth texture (if true) or color texture (if false)
     */
    GLuint GetTexture(bool depth = false);

    glm::ivec2 GetSize();

private:
    Shader* shader;

    glm::vec2 pixelsize;
    glm::ivec2 size;

    float data[20] = 
    {
        -1, 1, 0, 0, 1,
        -1, -1, 0, 0, 0,
        1, -1, 0, 1, 0,
        1, 1, 0, 1, 1
    };

    GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
    GLuint vao, vbo, fbo, ebo, texture = 0, depth = 0;
};
