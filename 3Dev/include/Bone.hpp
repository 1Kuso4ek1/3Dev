#pragma once
#include "Node.hpp"

class Bone : public Node
{
public:
    Bone(int id, const std::string& name, glm::mat4 offset);

    void SetTransform(const rp3d::Transform& tr) override;

    void SetPosition(const rp3d::Vector3& pos);
    void SetOrientation(const rp3d::Quaternion& orient);
    void SetSize(const rp3d::Vector3& size) override;

    void Move(const rp3d::Vector3& vec);
    void Rotate(const rp3d::Quaternion& quat);
    void Expand(const rp3d::Vector3& vec);

    void SavePoseAsIdle();

    int GetID();
    std::string GetName();

    rp3d::Vector3 GetPosition();
    rp3d::Quaternion GetOrientation();
    rp3d::Vector3 GetSize() override;
    
    glm::mat4 GetOffset();
    rp3d::Transform GetTransform() override;
    rp3d::Transform GetIdle();

    Json::Value Serialize();
    void Deserialize(Json::Value data);

private:
    int id = 0;
    std::string name = "";

    rp3d::Transform transform, idle;
    glm::mat4 offset;
    rp3d::Vector3 size;
};
