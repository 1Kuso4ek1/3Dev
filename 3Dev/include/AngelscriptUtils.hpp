#pragma once
#include <scriptbuilder.h>
#include <scriptstdstring.h>
#include <scriptarray.h>
#include <aswrappedcall.h>
#include <scriptmath.h>

static void MessageCallback(const asSMessageInfo *msg, void *param)
{
    switch(msg->type)
    {
    case asMSGTYPE_ERROR:
        Log::Write(std::string(msg->section) + " (" + std::to_string(msg->row) + ", "
                    + std::to_string(msg->col) + "): " + std::string(msg->message), Log::Type::Error);
        break;
    case asMSGTYPE_WARNING:
        Log::Write(std::string(msg->section) + " (" + std::to_string(msg->row) + ", "
                    + std::to_string(msg->col) + "): " + std::string(msg->message), Log::Type::Warning);
        break;
    case asMSGTYPE_INFORMATION:
        Log::Write(std::string(msg->section) + " (" + std::to_string(msg->row) + ", "
                    + std::to_string(msg->col) + "): " + std::string(msg->message), Log::Type::Info);
        break;
    }
}

template<class T>
class Group
{
public:
    Group(std::vector<T*> objects) : objects(objects) {}

    uint32_t Size() { return objects.size(); }

    T* operator[](uint32_t index) { return objects[index]; }

private:
    std::vector<T*> objects;
};

template<class T>
static void MakeType(T* memory) { new(memory) T(); }

template<class T>
static void CopyType(const T& other, T* memory) { new(memory) T(other); }

template<class T>
static void DestroyType(T* memory) { memory->~T(); }

template<class T>
static T& AssignType(const T& src, T* dst) { return *dst = src; }

template<class T>
static T* TypeFactory() { return new T(); }

static void MakeVector3(float x, float y, float z, rp3d::Vector3* vec) { new(vec) rp3d::Vector3(x, y, z); }
static rp3d::Vector3 AddVector3(const rp3d::Vector3& r, rp3d::Vector3* l) { return r + *l; }

static void MakeQuaternion(float x, float y, float z, float w, rp3d::Quaternion* q) { new(q) rp3d::Quaternion(x, y, z, w); }

static void MakeAABB(const rp3d::Vector3& min, const rp3d::Vector3& max, rp3d::AABB* aabb) { new(aabb) rp3d::AABB(min, max); }
static rp3d::AABB& AssignAABB(const rp3d::AABB& src, rp3d::AABB* dst) { dst->mergeWithAABB(src); return *dst; }

static void MakeDefaultRandomEngine(unsigned int seed, std::default_random_engine* eng) { new(eng) std::default_random_engine(seed); }

static double GenerateRandomNumber(double min, double max, std::default_random_engine* eng)
{
    return std::uniform_real_distribution<double>(min, max)(*eng);
}

static void MakeRay(const rp3d::Vector3& start, const rp3d::Vector3& end, rp3d::Ray* ray) { new(ray) rp3d::Ray(start, end); }

static rp3d::RaycastInfo& AssignRaycastInfo(const rp3d::RaycastInfo& src, rp3d::RaycastInfo* dst)
{
    dst->body = src.body;
    dst->collider = src.collider;
    dst->hitFraction = src.hitFraction;
    dst->meshSubpart = src.meshSubpart;
    dst->triangleIndex = src.triangleIndex;
    dst->worldNormal = src.worldNormal;
    dst->worldPoint = src.worldPoint;
    return *dst;
}

static void MakeHingeJointInfo(rp3d::RigidBody* r, rp3d::RigidBody* r1, const rp3d::Vector3& v, const rp3d::Vector3& v1, rp3d::HingeJointInfo* info)
{
    new(info) rp3d::HingeJointInfo(r, r1, v, v1);
}
