#pragma once
#define NOMINMAX

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <assimp/BaseImporter.h>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/ai_assert.h>
#include <assimp/texture.h>
#include <assimp/Vertex.h>
#include <assimp/Bitmap.h>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <reactphysics3d/reactphysics3d.h>

#define GL_CLAMP_TO_EDGE 0x812F

const double pi = 3.141592653;

constexpr glm::vec3 toglm(const rp3d::Vector3& vec) { return glm::vec3(vec.x, vec.y, vec.z); }
constexpr glm::vec3 toglm(const aiVector3D& vec) { return glm::vec3(vec.x, vec.y, vec.z); }
constexpr glm::vec2 toglm(const aiVector2D& vec) { return glm::vec2(vec.x, vec.y); }
constexpr glm::quat toglm(const rp3d::Quaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }