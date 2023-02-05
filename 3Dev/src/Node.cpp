#include <Node.hpp>

void Node::SetParent(Node* parent)
{
    this->parent = parent;
}

void Node::AddChild(Node* child)
{
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
        tr = tr * Node::GetFinalTransform(node->GetParent(), node->GetParent()->GetTransform());
    return tr;
}
