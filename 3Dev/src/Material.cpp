#include <Material.h>

void Material::SetParameters(float parameters[], GLenum type)
{
	mat[type] = parameters;
}

void Material::SetMap(std::unordered_map<GLenum, float*> m)
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
		glMaterialfv(GL_FRONT_AND_BACK, i.first, i.second);
	}
}

std::unordered_map<GLenum, float*> Material::GetMap()
{
	return mat;
}
