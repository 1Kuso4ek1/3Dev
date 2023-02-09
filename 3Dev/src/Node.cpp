#include <Node.hpp>

void Node::SetParent(Node* parent)
{
    if(this->parent == parent)
        this->parent = nullptr;
    else
        this->parent = parent;
}

void Node::AddChild(Node* child)
{
    auto it = std::find(children.begin(), children.end(), child);
    if(it != children.end())
        children.erase(it);
    else
        children.push_back(child);
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
