#pragma once
#include "Utils.hpp"

class Node
{
public:
    void SetParent(Node* parent);
    void AddChild(Node* child);

    Node* GetParent();
    std::vector<Node*> GetChildren();

    virtual rp3d::Transform GetTransform() = 0;
    
    static rp3d::Transform GetFinalTransform(Node* node, rp3d::Transform tr = rp3d::Transform::identity());

protected:
    Node* parent = nullptr;
    std::vector<Node*> children;
};
