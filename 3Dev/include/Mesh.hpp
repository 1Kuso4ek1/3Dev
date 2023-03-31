#pragma once
#include "Utils.hpp"
#include "Bone.hpp"

struct Vertex
{
    Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 uv) : position(pos), normal(norm), uv(uv) {}

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    glm::ivec4 ids = glm::ivec4(0);
    glm::vec4 weights = glm::vec4(0.0);
};

/*
 * This class stores data of the mesh (vertices and indices)
 */
class Mesh
{
public:
    /*
     * Basic constructor
     * @param data array of the vertices
     * @param indices array of mesh indices
     * @param aabb bounding box of the mesh
     */
    Mesh(std::vector<Vertex> data, std::vector<GLuint> indices, aiAABB aabb, glm::mat4 transformation = glm::mat4(1.0));
    Mesh() {}
    ~Mesh();

    // Simple draw fucntion without any shaders
    void Draw();

    // @return reference to the vertex data
    std::vector<Vertex>& GetData();

    // @return reference to the indices
    std::vector<GLuint>& GetIndices();

    glm::mat4 GetTransformation();

    // @return bounding box of this mesh
    aiAABB GetAABB();

    void CreateCube();

private:
    void SetupBuffers();

    aiAABB aabb;

    GLuint vao, vbo, ebo;

    glm::mat4 transformation;

    std::vector<Vertex> data;
	std::vector<GLuint> indices;
};
