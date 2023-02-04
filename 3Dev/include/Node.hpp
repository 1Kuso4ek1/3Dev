#pragma once
#include "Utils.hpp"

class Node
{
public:
    void SetParent(Node* parent);
    void AddChild(Node* child);

    Node* GetParent();
    virtual rp3d::Transform GetTransform() = 0;

protected:
    Node* parent = nullptr;
    std::vector<Node*> children;
};
