#include <Bone.hpp>

Bone::Bone(int id, std::string name, glm::mat4 offset) : id(id), name(name)
{
    this->offset = offset;
    size = rp3d::Vector3(1, 1, 1);
}

void Bone::SetTransform(const rp3d::Transform& tr)
{
    transform = tr;
}

void Bone::SetPosition(const rp3d::Vector3& pos)
{
    SavePoseAsIdle();
    transform.setPosition(pos);
}

void Bone::SetOrientation(const rp3d::Quaternion& orient)
{
    SavePoseAsIdle();
    transform.setOrientation(orient);
}

void Bone::SetSize(const rp3d::Vector3& size)
{
    SavePoseAsIdle();
    this->size = size;
}

void Bone::Move(const rp3d::Vector3& vec)
{
    SavePoseAsIdle();
    transform.setPosition(transform.getPosition() + vec);
}

void Bone::Rotate(const rp3d::Quaternion& quat)
{
    SavePoseAsIdle();
    transform.setOrientation(quat * transform.getOrientation());
}

void Bone::Expand(const rp3d::Vector3& vec)
{
    SavePoseAsIdle();
    size += vec;
}

void Bone::SavePoseAsIdle()
{
    idle = transform;
}

int Bone::GetID()
{
    return id;
}

std::string Bone::GetName()
{
    return name;
}

rp3d::Vector3 Bone::GetPosition()
{
    return transform.getPosition();
}

rp3d::Quaternion Bone::GetOrientation()
{
    return transform.getOrientation();
}

rp3d::Vector3 Bone::GetSize()
{
    return size;
}

glm::mat4 Bone::GetOffset()
{
    return offset;
}

rp3d::Transform Bone::GetTransform()
{
    return transform;
}

rp3d::Transform Bone::GetIdle()
{
    return idle;
}

Json::Value Bone::Serialize()
{
    Json::Value data;

    auto pos = idle.getPosition();
    auto orient = idle.getOrientation();

    data["position"]["x"] = pos.x;
    data["position"]["y"] = pos.y;
    data["position"]["z"] = pos.z;

    data["orientation"]["x"] = orient.x;
    data["orientation"]["y"] = orient.y;
    data["orientation"]["z"] = orient.z;
    data["orientation"]["w"] = orient.w;

    data["size"]["x"] = size.x;
    data["size"]["y"] = size.y;
    data["size"]["z"] = size.z;

    return data;
}

void Bone::Deserialize(Json::Value data)
{
    rp3d::Vector3 pos, size;
    rp3d::Quaternion orient;

    pos.x = data["position"]["x"].asFloat();
    pos.y = data["position"]["y"].asFloat();
    pos.z = data["position"]["z"].asFloat();

    orient.x = data["orientation"]["x"].asFloat();
    orient.y = data["orientation"]["y"].asFloat();
    orient.z = data["orientation"]["z"].asFloat();
    orient.w = data["orientation"]["w"].asFloat();

    size.x = data["size"]["x"].asFloat();
    size.y = data["size"]["y"].asFloat();
    size.z = data["size"]["z"].asFloat();

    transform.setPosition(pos);
    transform.setOrientation(orient);
    this->size = size;

    SavePoseAsIdle();
}
