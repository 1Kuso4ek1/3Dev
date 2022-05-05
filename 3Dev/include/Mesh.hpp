#pragma once
#include "3Dev.hpp"

struct Vertex
{
    Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 uv) : position(pos), normal(norm), uv(uv) {}

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    glm::ivec4 ids = glm::ivec4(0);
    glm::vec4 weights = glm::vec4(0.0);
};

struct Bone
{
    int id = 0;
    std::string name = "";
    glm::mat4 offset = glm::mat4(1.0);

    std::vector<Bone> children = {};
};

/*
 * This class stores data of the mesh (vertices, indices and bone data)
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
    Mesh(std::vector<Vertex> data, std::vector<GLuint> indices, aiAABB aabb, std::vector<Bone> bones = {}, glm::mat4 transformation = glm::mat4(1.0));
    ~Mesh();

    // Simple draw fucntion without any shaders
    void Draw();

    // @return reference to the vertex data
    std::vector<Vertex>& GetData();

    // @return reference to the indices
    std::vector<GLuint>& GetIndices();

    std::vector<Bone>& GetBones();

    std::vector<glm::mat4>& GetPose();

    glm::mat4 GetTransformation();

    // @return bounding box of this mesh
    aiAABB GetAABB();

private:
    int BonesCount(Bone& b);

    aiAABB aabb;

    GLuint vao, vbo, ebo;

    glm::mat4 transformation;

    std::vector<Vertex> data;
	std::vector<GLuint> indices;
    std::vector<Bone> bones;
    std::vector<glm::mat4> pose;
};
