#pragma once
#include "3Dev.h"
#include "Shader.h"

/*
 * This class is used to replace standart OpenGL matrices.
 */
class Matrices
{
public:
    // Default constructor
    Matrices();

    /*
     * Used to scale a model matrix
     * @param size new size of a model matrix
     */
    void Scale(glm::vec3 size);
    
    /*
     * Used to translate a model matrix
     * @param pos new position of a model matrix
     */
    void Translate(glm::vec3 pos);

    /*
     * Used to rotate a model matrix
     * @param angle rotation angle of a model matrix
     * @param axis the axis around which it will be rotated
     */
    void Rotate(float angle, glm::vec3 axis);

    // Push the new identity matrices
    void PushMatrix();

    // Pop the matrices
    void PopMatrix();

    /*
     * Used to read/write model matrix
     * @return reference to the model matrix
     */
    glm::mat4& GetModel();

    /*
     * Used to read/write view matrix
     * @return reference to the view matrix
     */
    glm::mat4& GetView();

    /*
     * Used to read/write projection matrix
     * @return reference to the projection matrix
     */
    glm::mat4& GetProjection();

    /*
     * Used to update all matrix uniforms in the shader
     * @param shader pointer to the shader
     */
    void UpdateShader(Shader* shader);

private:
    std::vector<glm::mat4> model, view, projection;    
};
