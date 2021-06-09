#include <Material.h>

void Material::SetParameters(float parameters[], GLenum type)
{
	mat[type] = parameters;
}

void Material::Activate()
{
	for(auto i : mat)
	{
		glMaterialfv(GL_FRONT_AND_BACK, i.first, i.second);
	}
}
