#pragma once
#include "3Dev.h"

struct Vertex
{
    Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 uv) : position(pos), normal(norm), uv(uv) {}

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
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
    Mesh(std::vector<Vertex> data, std::vector<GLuint> indices, aiAABB aabb);
    ~Mesh();

    // Simple draw fucntion without any shaders
    void Draw();

    // @return reference to the vertex data
    std::vector<Vertex>& GetData();

    // @return reference to the indices
    std::vector<GLuint>& GetIndices();

    // @return bounding box of this mesh
    aiAABB GetAABB();

private:
    aiAABB aabb;

    GLuint vao, vbo, ebo;

    std::vector<Vertex> data;
	std::vector<GLuint> indices;
};