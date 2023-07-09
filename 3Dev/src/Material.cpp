#include <Material.hpp>

Material::Material()
{
    SetParameter(Renderer::GetInstance()->GetTexture(Renderer::TextureType::Irradiance), Type::Irradiance);
    SetParameter(Renderer::GetInstance()->GetTexture(Renderer::TextureType::Prefiltered), Type::PrefilteredMap);
    SetParameter(Renderer::GetInstance()->GetTexture(Renderer::TextureType::LUT), Type::LUT);
}

Material::Material(std::vector<std::pair<std::variant<glm::vec3, GLuint>, Type>> parameters) : parameters(parameters)
{
	if(!Contains(Type::Cubemap) && !Contains(Type::Environment))
	{
		SetParameter(Renderer::GetInstance()->GetTexture(Renderer::TextureType::Irradiance), Type::Irradiance);
		SetParameter(Renderer::GetInstance()->GetTexture(Renderer::TextureType::Prefiltered), Type::PrefilteredMap);
		SetParameter(Renderer::GetInstance()->GetTexture(Renderer::TextureType::LUT), Type::LUT);
	}
}

void Material::SetParameter(std::variant<glm::vec3, GLuint> parameter, Type type)
{
    if(!Contains(type))
        parameters.emplace_back(parameter, type);
    else
        std::find_if(parameters.begin(), parameters.end(), [&](auto& a)
                    {
                        return a.second == type;
                    })->first = parameter;
}

void Material::UpdateShader(Shader* shader)
{
	bool prefilteredMap = false, irradiance = false;
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
			if(parameters[i].second == Material::Type::Irradiance ||
			   parameters[i].second == Material::Type::PrefilteredMap)
				glBindTexture(GL_TEXTURE_CUBE_MAP, param1);
			else glBindTexture(GL_TEXTURE_2D, param1);
		}

		switch(parameters[i].second)
		{
		case Material::Type::Color:
			if(param1 != 0)
				shader->SetUniform1i("albedo", i);
			shader->SetUniform3f("nalbedo", (param1 == 0 ? param0.x : -1), param0.y, param0.z);
			break;
		case Material::Type::Normal:
			shader->SetUniform1i("nnormalMap", 1);
			shader->SetUniform1i("normalMap", i);
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
			shader->SetUniform3f("nemission", (param1 == 0 ? param0.x : -1), param0.y, param0.z);
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
			if(param1 != 0)
			{
				shader->SetUniform1i("irradiance", i);
				irradiance = true;
			}
			shader->SetUniform3f("nirradiance", (param1 == 0 ? param0.x : -1), param0.y, param0.z);
			break;
		case Material::Type::PrefilteredMap:
			shader->SetUniform1i("prefilteredMap", i);
			prefilteredMap = true;
			break;
		case Material::Type::LUT:
			shader->SetUniform1i("lut", i);
			break;
		}
	}
	if(!irradiance || !prefilteredMap)
	{
		glActiveTexture(GL_TEXTURE0 + parameters.size());
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		if(!irradiance) shader->SetUniform1i("irradiance", parameters.size());
		if(!prefilteredMap) shader->SetUniform1i("prefilteredMap", parameters.size());
	}
}

void Material::ResetShader(Shader* shader)
{
	shader->SetUniform3f("nalbedo", 0, 0, 0);
	shader->SetUniform1i("albedo", 0);
	shader->SetUniform1i("nnormalMap", 0);
	shader->SetUniform1i("normalMap", 0);
	shader->SetUniform1i("nao", 0);
	shader->SetUniform1i("ao", 0);
	shader->SetUniform1f("nmetalness", 0);
	shader->SetUniform1i("metalness", 0);
	shader->SetUniform1f("nemission", 0);
	shader->SetUniform1i("emission", 0);
	shader->SetUniform1f("nroughness", 0);
	shader->SetUniform1i("roughness", 0);
	shader->SetUniform1f("nopacity", 1);
	shader->SetUniform1i("opacity", 0);
	shader->SetUniform1i("cubemap", 0);
	shader->SetUniform1i("environment", 0);
	shader->SetUniform1i("irradiance", 0);
	shader->SetUniform3f("nirradiance", 0, 0, 0);
	shader->SetUniform1i("prefilteredMap", 0);
	shader->SetUniform1i("lut", 0);
}

void Material::UpdateShaderEnvironment(Shader* shader)
{
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Renderer::GetInstance()->GetTexture(Renderer::TextureType::Irradiance));
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Renderer::GetInstance()->GetTexture(Renderer::TextureType::Prefiltered));
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, Renderer::GetInstance()->GetTexture(Renderer::TextureType::LUT));

	shader->SetUniform3f("nirradiance", -1, 0, 0);
	shader->SetUniform1i("irradiance", 5);
	shader->SetUniform1i("prefilteredMap", 6);
	shader->SetUniform1i("lut", 7);
}

void Material::GetEnvironmentFromRenderer()
{
	SetParameter(Renderer::GetInstance()->GetTexture(Renderer::TextureType::Irradiance), Type::Irradiance);
	SetParameter(Renderer::GetInstance()->GetTexture(Renderer::TextureType::Prefiltered), Type::PrefilteredMap);
}

bool Material::Contains(Type type)
{
	return std::find_if(parameters.begin(), parameters.end(), [&](auto& a)
				{
					return a.second == type;
				}) != parameters.end();
}

Json::Value Material::Serialize()
{
	Json::Value data;

	for(auto& i : parameters)
	{
		switch(i.second)
		{
		case Material::Type::Color:
			if(std::holds_alternative<glm::vec3>(i.first))
			{
				data["color"]["r"] = std::get<0>(i.first).x;
				data["color"]["g"] = std::get<0>(i.first).y;
				data["color"]["b"] = std::get<0>(i.first).z;
			}
			else data["color"]["filename"] = TextureManager::GetInstance()->GetFilename(std::get<1>(i.first));
			break;
		case Material::Type::Normal:
			data["normal"]["filename"] = TextureManager::GetInstance()->GetFilename(std::get<1>(i.first));
			break;
		case Material::Type::AmbientOcclusion:
			data["ao"]["filename"] = TextureManager::GetInstance()->GetFilename(std::get<1>(i.first));
			break;
		case Material::Type::Metalness:
			if(std::holds_alternative<glm::vec3>(i.first))
				data["metalness"]["value"] = std::get<0>(i.first).x;
			else data["metalness"]["filename"] = TextureManager::GetInstance()->GetFilename(std::get<1>(i.first));
			break;
		case Material::Type::Emission:
			if(std::holds_alternative<glm::vec3>(i.first))
			{
				data["emission"]["r"] = std::get<0>(i.first).x;
				data["emission"]["g"] = std::get<0>(i.first).y;
				data["emission"]["b"] = std::get<0>(i.first).z;
			}
			else data["emission"]["filename"] = TextureManager::GetInstance()->GetFilename(std::get<1>(i.first));
			break;
		case Material::Type::Roughness:
			if(std::holds_alternative<glm::vec3>(i.first))
				data["roughness"]["value"] = std::get<0>(i.first).x;
			else data["roughness"]["filename"] = TextureManager::GetInstance()->GetFilename(std::get<1>(i.first));
			break;
		case Material::Type::Opacity:
			if(std::holds_alternative<glm::vec3>(i.first))
				data["opacity"]["value"] = std::get<0>(i.first).x;
			else data["opacity"]["filename"] = TextureManager::GetInstance()->GetFilename(std::get<1>(i.first));
			break;
		}
	}

	return data;
}

void Material::Deserialize(Json::Value data)
{
    parameters.clear();
	if(!data["color"]["filename"].empty())
        parameters.push_back({ TextureManager::GetInstance()->LoadTexture(data["color"]["filename"].asString()), Type::Color });
    else
	{
        parameters.push_back({ glm::vec3(data["color"]["r"].asDouble(),
                                         data["color"]["g"].asDouble(),
                                         data["color"]["b"].asDouble()), Type::Color });
	}
	if(!data["normal"]["filename"].empty())
        parameters.push_back({ TextureManager::GetInstance()->LoadTexture(data["normal"]["filename"].asString()), Type::Normal });
    if(!data["ao"]["filename"].empty())
        parameters.push_back({ TextureManager::GetInstance()->LoadTexture(data["ao"]["filename"].asString()), Type::AmbientOcclusion });
    if(!data["metalness"]["filename"].empty())
        parameters.push_back({ TextureManager::GetInstance()->LoadTexture(data["metalness"]["filename"].asString()), Type::Metalness });
    else parameters.push_back({ glm::vec3(data["metalness"]["value"].asDouble()), Type::Metalness});
    if(!data["emission"]["filename"].empty())
        parameters.push_back({ TextureManager::GetInstance()->LoadTexture(data["emission"]["filename"].asString()), Type::Emission });
    else
	{
        parameters.push_back({ glm::vec3(data["emission"]["r"].asDouble(),
                                         data["emission"]["g"].asDouble(),
                                         data["emission"]["b"].asDouble()), Type::Emission });
	}
	if(!data["roughness"]["filename"].empty())
        parameters.push_back({ TextureManager::GetInstance()->LoadTexture(data["roughness"]["filename"].asString()), Type::Roughness});
    else parameters.push_back({ glm::vec3(data["roughness"]["value"].asDouble()), Type::Roughness});
    if(!data["opacity"]["filename"].empty())
        parameters.push_back({ TextureManager::GetInstance()->LoadTexture(data["opacity"]["filename"].asString()), Type::Opacity });
    else parameters.push_back({ glm::vec3(data["opacity"]["value"].asDouble()), Type::Opacity});

    GetEnvironmentFromRenderer();
    SetParameter(Renderer::GetInstance()->GetTexture(Renderer::TextureType::LUT), Type::LUT);
}

std::variant<glm::vec3, GLuint> Material::GetParameter(Type type)
{
    auto ret = std::find_if(parameters.begin(), parameters.end(), [&](auto& a)
                    {
                        return a.second == type;
                    });
    return ret->first;
}

std::vector<std::pair<std::variant<glm::vec3, GLuint>, Material::Type>>& Material::GetParameters()
{
	return parameters;
}

bool Material::operator==(Material& r)
{
	if(parameters.size() != r.GetParameters().size()) return false;
	for(int i = 0; i < parameters.size(); i++)
	{
		if(parameters[i] != r.GetParameters()[i])
			return false;
	}
	return true;
}

bool Material::operator!=(Material& r)
{
	return !(*this == r);
}
