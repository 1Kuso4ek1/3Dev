#include <Material.h>

Material::Material(float shininess, std::vector<std::pair<GLuint, Material::TexType>> textures) : shininess(shininess), textures(textures) {}

void Material::SetShininess(float shininess)
{
	this->shininess = shininess;
}

float Material::GetShininess()
{
	return shininess;
}

std::vector<std::pair<GLuint, Material::TexType>>& Material::GetTextures()
{
	return textures;
}
