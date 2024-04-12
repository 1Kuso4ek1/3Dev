#pragma once
#include "Utils.hpp"

class Node
{
public:
    void SetParent(Node* parent);
    virtual void SetTransform(const rp3d::Transform& transform);
    virtual void SetSize(const rp3d::Vector3& size);
    virtual void AddChild(Node* child);

    virtual void Draw();

    Node* GetParent();
    std::vector<Node*> GetChildren();

    virtual rp3d::Transform GetTransform() = 0;
    virtual rp3d::RigidBody* GetRigidBody();
    virtual rp3d::Vector3 GetSize();
    
    static rp3d::Transform GetFinalTransform(Node* node, rp3d::Transform tr = rp3d::Transform::identity());

protected:
    Node* parent = nullptr;
    std::vector<Node*> children;
};
