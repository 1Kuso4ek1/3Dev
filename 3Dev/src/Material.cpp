#include <Material.h>

Material::Material()
{
	mat = {
		{ GL_AMBIENT, { 0.2, 0.2, 0.2, 1 } },
		{ GL_DIFFUSE, { 0.8, 0.8, 0.8, 1 } },
		{ GL_SPECULAR, { 0, 0, 0, 1 } },
		{ GL_EMISSION, { 0, 0, 0, 1 } },
		{ GL_SHININESS, { 0, 0, 0, 0 } }
	};
}

void Material::SetParameters(std::vector<float> parameters, GLenum type)
{
	mat[type] = parameters;
}

void Material::SetMap(std::unordered_map<GLenum, std::vector<float>> m)
{
	mat = m;
}

void Material::operator=(Material& right)
{
	mat = right.GetMap();
}

void Material::Activate()
{
	for(auto i : mat)
	{
		glMaterialfv(GL_FRONT_AND_BACK, i.first, &i.second[0]);
	}
}

std::unordered_map<GLenum, std::vector<float>> Material::GetMap()
{
	return mat;
}
