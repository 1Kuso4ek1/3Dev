#pragma once
#include "Utils.hpp"
#include <scriptbuilder.h>
#include <scriptstdstring.h>
#include <scriptarray.h>
#include <aswrappedcall.h>
#include <scriptmath.h>
#include <datetime.h>
#include <scriptdictionary.h>
#include <scriptfile.h>

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
static rp3d::Vector3 AddVector3(const rp3d::Vector3& r, rp3d::Vector3* l) { return *l + r; }
static rp3d::Vector3 SubVector3(const rp3d::Vector3& r, rp3d::Vector3* l) { return *l - r; }
static rp3d::Vector3 MulVector3(float r, rp3d::Vector3* l) { return *l * r; }
static rp3d::Vector3 DivVector3(float r, rp3d::Vector3* l) { return *l / r; }

static void MakeQuaternion(float x, float y, float z, float w, rp3d::Quaternion* q) { new(q) rp3d::Quaternion(x, y, z, w); }
static rp3d::Quaternion AddQuaternion(const rp3d::Quaternion& r, rp3d::Quaternion* l) { return *l + r; }
static rp3d::Quaternion SubQuaternion(const rp3d::Quaternion& r, rp3d::Quaternion* l) { return *l - r; }
static rp3d::Quaternion MulQuaternion(const rp3d::Quaternion& r, rp3d::Quaternion* l) { return *l * r; }

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

static void MakeBallAndSocketJointInfo(rp3d::RigidBody* r, rp3d::RigidBody* r1, const rp3d::Vector3& v, rp3d::BallAndSocketJointInfo* info)
{
    new(info) rp3d::BallAndSocketJointInfo(r, r1, v);
}

static void MakeFixedJointInfo(rp3d::RigidBody* r, rp3d::RigidBody* r1, const rp3d::Vector3& v, rp3d::FixedJointInfo* info)
{
    new(info) rp3d::FixedJointInfo(r, r1, v);
}

static void MakeSliderJointInfo(rp3d::RigidBody* r, rp3d::RigidBody* r1, const rp3d::Vector3& v, const rp3d::Vector3& v1, rp3d::SliderJointInfo* info)
{
    new(info) rp3d::SliderJointInfo(r, r1, v, v1);
}

static void MakeGui(sf::Window& window, tgui::Gui* gui) { new(gui) tgui::Gui(window); }

static void OnPress(asIScriptFunction& func, tgui::Button* button)
{
    button->onPress([&]()
    {
        auto context = (asIScriptContext*)func.GetEngine()->RequestContext();
        context->Prepare(&func);
        context->Execute();
        func.GetEngine()->ReturnContext(context);
    });
}

static void MakeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a, tgui::Color* color) { new(color) tgui::Color(r, g, b, a); }
static void MakeString(const std::string& str, tgui::String* string) { new(string) tgui::String(str); }
static void MakeDuration(const sf::Time& time, tgui::Duration* duration) { new(duration) tgui::Duration(time); }

template<class T>
static T* GetWidget(std::string name, tgui::Gui* gui) { return gui->get<T>(name).get(); }

static void SetMaterialParameter(rp3d::Vector3 value, Material::Type parameter, Material* material)
{
    material->SetParameter(toglm(value), parameter);
}

static rp3d::Vector3 GetMaterialParameter(Material::Type parameter, Material* material)
{
    if(material->Contains(parameter))
    {
        auto value = material->GetParameter(parameter);
        if(std::holds_alternative<glm::vec3>(value))
        {
            auto vec = std::get<0>(value);
            return rp3d::Vector3(vec.x, vec.y, vec.z);
        }
    }
    return rp3d::Vector3::zero();
}

static void SetPhysicalMaterial(const rp3d::Material& mat, rp3d::RigidBody* b)
{
    for(int i = 0; i < b->getNbColliders(); i++)    
        b->getCollider(i)->setMaterial(mat);
}

static rp3d::Vector3 EulerFromQuaternion(const rp3d::Quaternion& orientation)
{
    glm::vec3 euler = glm::eulerAngles(toglm(orientation));
    return rp3d::Vector3(euler.x, euler.y, euler.z);
}

static Shader* MakeShader(const std::string& str, const std::string& str1, bool load)
{
    return new Shader(str, str1, load);
}

static Node* CastLight(Light* light)
{
    return dynamic_cast<Node*>(light);
}

static Node* CastModel(Model* model)
{
    return dynamic_cast<Node*>(model);
}

static Node* CastCamera(Camera* camera)
{
    return dynamic_cast<Node*>(camera);
}

static Node* CastBone(Bone* bone)
{
    return dynamic_cast<Node*>(bone);
}

static Light* CastToLight(Node* node)
{
    return dynamic_cast<Light*>(node);
}

static Model* CastToModel(Node* node)
{
    return dynamic_cast<Model*>(node);
}

static Camera* CastToCamera(Node* node)
{
    return dynamic_cast<Camera*>(node);
}

static Bone* CastToBone(Node* node)
{
    return dynamic_cast<Bone*>(node);
}

static sf::IpAddress ResolveIp(std::string addr)
{
    auto ip = sf::IpAddress::resolve(std::string_view(addr));
    if(ip.has_value())
        return ip.value();
    return sf::IpAddress::Any;
}

static int Stoi(const std::string& str, int base) { return std::stoi(str, nullptr, base); }
static float Stof(const std::string& str) { return std::stof(str); }

static size_t AddListViewItem(const CScriptArray& array, tgui::ListView* listView)
{
    std::vector<tgui::String> data;
    for(asUINT i = 0; i < array.GetSize(); i++)
        data.push_back(*(std::string*)(array.At(i)));
    return listView->addItem(data);
}

static void InsertListViewItem(size_t index, const CScriptArray& array, tgui::ListView* listView)
{
    std::vector<tgui::String> data;
    for(asUINT i = 0; i < array.GetSize(); i++)
        data.push_back(*(std::string*)(array.At(i)));
    listView->insertItem(index, data);
}

static bool ChangeListViewItem(size_t index, const CScriptArray& array, tgui::ListView* listView)
{
    std::vector<tgui::String> data;
    for(asUINT i = 0; i < array.GetSize(); i++)
        data.push_back(*(std::string*)(array.At(i)));
    return listView->changeItem(index, data);
}

static rp3d::Quaternion LookAt(const rp3d::Vector3& eye, const rp3d::Vector3& center, const rp3d::Vector3& up)
{
    auto q = glm::conjugate(glm::toQuat(glm::lookAt(toglm(eye), toglm(center), toglm(up))));

    return rp3d::Quaternion(q.x, q.y, q.z, q.w);
}
