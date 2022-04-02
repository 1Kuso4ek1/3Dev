#include <Material.h>

Material::Material(float shininess, std::vector<std::pair<GLuint, Material::TexType>> textures) : shininess(shininess), textures(textures) {}

void Material::SetShininess(float shininess)
{
	this->shininess = shininess;
}

void Material::AddTexture(GLuint texture, TexType type)
{
	textures.emplace_back(texture, type);
}

void Material::UpdateShader(Shader* shader)
{
	bool cubemap = false;
	for(int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		if(textures[i].second != Material::TexType::Cubemap) glBindTexture(GL_TEXTURE_2D, textures[i].first);
		else glBindTexture(GL_TEXTURE_CUBE_MAP, textures[i].first);

		switch(textures[i].second)
		{
		case Material::TexType::Diffuse:
			shader->SetUniform1i("ndiff", 1);
			shader->SetUniform1i(std::string("diff"), i);
			break;
		case Material::TexType::NormalMap:
			shader->SetUniform1i("nnormalmap", 1);
			shader->SetUniform1i(std::string("normalmap"), i);
			break;
		case Material::TexType::AmbientOcclusion:
			shader->SetUniform1i("nao", 1);
			shader->SetUniform1i(std::string("ao"), i);
			break;
		case Material::TexType::Metalness:
			shader->SetUniform1i("nmetalness", 1);
			shader->SetUniform1i(std::string("metalness"), i);
			break;
		case Material::TexType::Emission:
			shader->SetUniform1i("nemission", 1);
			shader->SetUniform1i(std::string("emission"), i);
			break;
		case Material::TexType::Roughness:
			shader->SetUniform1i("nroughness", 1);
			shader->SetUniform1i(std::string("roughness"), i);
			break;
		case Material::TexType::Opacity:
			shader->SetUniform1i("nopacity", 1);
			shader->SetUniform1i(std::string("opacity"), i);
			break;
		case Material::TexType::Cubemap:
			shader->SetUniform1i(std::string("cubemap"), i);
			cubemap = true;
			break;
		}
	}
	if(!cubemap)
	{
		glActiveTexture(GL_TEXTURE0 + textures.size());
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		shader->SetUniform1i(std::string("cubemap"), textures.size());
	}
}

void Material::ResetShader(Shader* shader)
{
	shader->SetUniform1i("ndiff", 0);
	shader->SetUniform1i("diff", 0);
	shader->SetUniform1i("nnormalmap", 0);
	shader->SetUniform1i("normalmap", 0);
	shader->SetUniform1i("nao", 0);
	shader->SetUniform1i("ao", 0);
	shader->SetUniform1i("nmetalness", 0);
	shader->SetUniform1i("metalness", 0);
	shader->SetUniform1i("nemission", 0);
	shader->SetUniform1i("emission", 0);
	shader->SetUniform1i("nroughness", 0);
	shader->SetUniform1i("roughness", 0);
	shader->SetUniform1i("nopacity", 0);
	shader->SetUniform1i("opacity", 0);
	shader->SetUniform1i("cubemap", 0);
}

float Material::GetShininess()
{
	return shininess;
}

std::vector<std::pair<GLuint, Material::TexType>>& Material::GetTextures()
{
	return textures;
}
