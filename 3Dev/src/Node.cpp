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
