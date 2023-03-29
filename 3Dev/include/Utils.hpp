#pragma once
#define NOMINMAX

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <random>

#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Network.hpp>

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
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <reactphysics3d/reactphysics3d.h>

#include <Log.h>

#include <json/json.h>

#include <angelscript.h>

#define TGUI_USE_STD_FILESYSTEM

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-OpenGL3.hpp>
#include <unistd.h>

constexpr glm::vec3 toglm(const rp3d::Vector3& vec) { return glm::vec3(vec.x, vec.y, vec.z); }
constexpr glm::vec3 toglm(const aiVector3D& vec) { return glm::vec3(vec.x, vec.y, vec.z); }
constexpr glm::vec2 toglm(const aiVector2D& vec) { return glm::vec2(vec.x, vec.y); }
constexpr glm::quat toglm(const rp3d::Quaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }
constexpr glm::quat toglm(const aiQuaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }

static glm::mat4 toglm(const aiMatrix4x4& mat)
{
    glm::mat4 ret;
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
            ret[j][i] = mat[i][j];
    return ret;
}

static std::pair<int, float> TimeFraction(std::vector<float>& times, float dt)
{
    if(times.empty()) return { 1, 1.0 };

	int seg = 0;
	while(dt > times[seg]) seg++;
	float frac = (dt - times[seg - 1]) / (times[seg] - times[seg - 1]);

	return { seg, frac };
}

static std::pair<rp3d::Transform, rp3d::Vector3> ToRP3DTransform(const glm::mat4& tr)
{
    rp3d::Transform ret;
    rp3d::Vector3 size;

    glm::vec3 pos, scale, skew;
    glm::vec4 perspective;
    glm::quat orient;

    glm::decompose(tr, scale, orient, pos, skew, perspective);
    orient = glm::conjugate(orient);

    size = { scale.x, scale.y, scale.z };

    ret.setPosition({ pos.x, pos.y, pos.z });
    ret.setOrientation({ orient.x, orient.y, orient.z, orient.w });

    return { ret, size };
}
