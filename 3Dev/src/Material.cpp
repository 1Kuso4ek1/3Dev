#include <Material.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.hpp>

Material::Material() {}

Material::Material(std::vector<std::pair<std::variant<glm::vec3, GLuint>, Type>> parameters) : parameters(parameters) {}

void Material::AddParameter(std::variant<glm::vec3, GLuint> parameter, Type type)
{
	parameters.emplace_back(parameter, type);
}

void Material::UpdateShader(Shader* shader)
{
	bool cubemap = false;
	for(int i = 0; i < parameters.size(); i++)
	{
		glm::vec3 param0(-1.0);
		GLuint param1 = 0;

		if(std::holds_alternative<glm::vec3>(parameters[i].first))
			param0 = std::get<0>(parameters[i].first);
		else
			param1 = std::get<1>(parameters[i].first);
			
		if(param1 != 0)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			if(parameters[i].second != Material::Type::Cubemap) glBindTexture(GL_TEXTURE_2D, param1);
			else glBindTexture(GL_TEXTURE_CUBE_MAP, param1);
		}

		switch(parameters[i].second)
		{
		case Material::Type::Color:
			if(param1 != 0)
				shader->SetUniform1i("albedo", i);
			shader->SetUniform3f("nalbedo", (param1 == 0 ? param0.x : -1), param0.y, param0.z);
			break;
		case Material::Type::Normal:
			shader->SetUniform1i("nnormalmap", 1);
			shader->SetUniform1i("normalmap", i);
			break;
		case Material::Type::AmbientOcclusion:
			shader->SetUniform1i("nao", 1);
			shader->SetUniform1i("ao", i);
			break;
		case Material::Type::Metalness:
			if(param1 != 0)
				shader->SetUniform1i("metalness", i);
			shader->SetUniform1f("nmetalness", param1 == 0 ? param0.x : -1);
			break;
		case Material::Type::Emission:
			if(param1 != 0)
				shader->SetUniform1i("emission", i);
			shader->SetUniform1f("nemission", param1 == 0 ? param0.x : -1);
			break;
		case Material::Type::Roughness:
			if(param1 != 0)
				shader->SetUniform1i("roughness", i);
			shader->SetUniform1f("nroughness", param1 == 0 ? param0.x : -1);
			break;
		case Material::Type::Opacity:
			if(param1 != 0)
				shader->SetUniform1i("opacity", i);
			shader->SetUniform1f("nopacity", param1 == 0 ? param0.x : -1);
			break;
		case Material::Type::Environment:
			shader->SetUniform1i("environment", i);
			break;
		case Material::Type::Irradiance:
			shader->SetUniform1i("irradiance", i);
			break;
		case Material::Type::Cubemap:
			shader->SetUniform1i("cubemap", i);
			cubemap = true;
			break;
		}
	}
	if(!cubemap)
	{
		glActiveTexture(GL_TEXTURE0 + parameters.size());
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		shader->SetUniform1i(std::string("cubemap"), parameters.size());
	}
}

void Material::ResetShader(Shader* shader)
{
	shader->SetUniform3f("nalbedo", -1, -1, -1);
	shader->SetUniform1i("albedo", 0);
	shader->SetUniform1i("nnormalmap", 0);
	shader->SetUniform1i("normalmap", 0);
	shader->SetUniform1i("nao", 0);
	shader->SetUniform1i("ao", 0);
	shader->SetUniform1f("nmetalness", -1);
	shader->SetUniform1i("metalness", 0);
	shader->SetUniform1f("nemission", -1);
	shader->SetUniform1i("emission", 0);
	shader->SetUniform1f("nroughness", -1);
	shader->SetUniform1i("roughness", 0);
	shader->SetUniform1f("nopacity", 1);
	shader->SetUniform1i("opacity", 0);
	shader->SetUniform1i("cubemap", 0);
	shader->SetUniform1i("environment", 0);
	shader->SetUniform1i("irradiance", 0);
}

std::vector<std::pair<std::variant<glm::vec3, GLuint>, Material::Type>>& Material::GetParameters()
{
	return parameters;
}
