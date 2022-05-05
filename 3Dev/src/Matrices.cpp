#include <Matrices.hpp>

Matrices::Matrices()
{
    model.push_back(glm::mat4(1.0));
    view.push_back(glm::mat4(1.0));
    projection.push_back(glm::mat4(1.0));
}

void Matrices::Scale(glm::vec3 size)
{
    model.back() = glm::scale(model.back(), size);
}

void Matrices::Translate(glm::vec3 pos)
{
    model.back() = glm::translate(model.back(), pos);
}

void Matrices::Rotate(float angle, glm::vec3 axis)
{
    model.back() = glm::rotate(model.back(), angle, axis);
}

void Matrices::PushMatrix()
{
    model.push_back(model.back());
    view.push_back(view.back());
}

void Matrices::PopMatrix()
{
    if(model.size() > 1) model.pop_back();
    if(view.size() > 1) view.pop_back();
}

glm::mat4& Matrices::GetModel()
{
    return model.back();
}

glm::mat4& Matrices::GetView()
{
    return view.back();
}

glm::mat4& Matrices::GetProjection()
{
    return projection.back();
}

void Matrices::UpdateShader(Shader* shader)
{
    shader->SetUniformMatrix4("model", model.back());
    shader->SetUniformMatrix4("view", view.back());
    shader->SetUniformMatrix4("projection", projection.back());
}
