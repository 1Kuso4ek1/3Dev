#include <Node.hpp>

void Node::SetParent(Node* parent)
{
    if(this->parent == parent)
        this->parent = nullptr;
    else
        this->parent = parent;
}

void Node::SetTransform(const rp3d::Transform& transform)
{
    return;
}

void Node::SetSize(const rp3d::Vector3& size)
{
    return;
}

void Node::AddChild(Node* child)
{
    auto it = std::find(children.begin(), children.end(), child);
    if(it != children.end())
        children.erase(it);
    else
        children.push_back(child);
}

void Node::Draw(Node* cam, std::vector<Node*> lights, bool transparencyPass)
{
    for(auto i : children)
        i->Draw(cam, lights, transparencyPass);
}

rp3d::RigidBody* Node::GetRigidBody()
{
    return nullptr;
}

rp3d::Vector3 Node::GetSize()
{
    return rp3d::Vector3(1, 1, 1);
}

Node* Node::GetParent()
{
    return parent;
}

std::vector<Node*> Node::GetChildren()
{
    return children;
}

rp3d::Transform Node::GetFinalTransform(Node* node, rp3d::Transform tr)
{
    if(node->GetParent())
        tr = Node::GetFinalTransform(node->GetParent(), node->GetParent()->GetTransform()) * tr;
    return tr;
}
