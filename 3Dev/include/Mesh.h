#pragma once
#include "3Dev.h"

struct Vertex
{
    Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 uv) : position(pos), normal(norm), uv(uv) {}

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

class Mesh
{
public:
    Mesh(std::vector<Vertex> data, std::vector<GLuint> indices, aiAABB aabb);
    ~Mesh();

    void Draw();

    std::vector<Vertex> GetData();
    std::vector<GLuint> GetIndices();
    aiAABB GetAABB();

private:
    aiAABB aabb;

    GLuint vao, vbo, ebo;

    std::vector<Vertex> data;
	std::vector<GLuint> indices;
};