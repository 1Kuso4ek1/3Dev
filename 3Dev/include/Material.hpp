#pragma once
#include "Utils.hpp"
#include "Shader.hpp"
#include "Renderer.hpp"
#include <variant>

/*
 * This class contains various material parameters
 */
class Material
{
public:
	enum class Type
	{
		Color,
		Normal,
		AmbientOcclusion,
		Metalness,
		Emission,
		EmissionStrength,
		Roughness,
		Opacity,
		Cubemap,
		Environment,
		Irradiance,
		PrefilteredMap,
		LUT
	};

	// Default constructor
	Material();

	/*
	 * Basic constructor
	 * @param textures vector with all parameters
	 */
	Material(std::vector<std::pair<std::variant<glm::vec3, GLuint>, Type>> parameters);

	/*
	 * @param parameter new parameter
	 * @param type type of a new parameter
	 */
	void SetParameter(std::variant<glm::vec3, GLuint> parameter, Type type);

	void UpdateShader(Shader* shader);
	static void UpdateShaderEnvironment(Shader* shader);
	void ResetShader(Shader* shader);
	void GetEnvironmentFromRenderer();
	void LoadTextures();

	bool Contains(Type type);

	Json::Value Serialize();

	void Deserialize(Json::Value data, bool loadTextures = true);

	std::variant<glm::vec3, GLuint> GetParameter(Type type);

	// @return reference to all parameters
	std::vector<std::pair<std::variant<glm::vec3, GLuint>, Type>>& GetParameters();

	bool operator==(Material& r);
	bool operator!=(Material& r);

private:
	bool texturesLoaded = false;

	std::unordered_map<Type, std::string> textureFilenames;

	std::vector<std::pair<std::variant<glm::vec3, GLuint>, Type>> parameters =
	{
		{ glm::vec3(1.0), Type::Color },
		{ glm::vec3(0.5), Type::Metalness },
		{ glm::vec3(0.0), Type::Emission },
		{ glm::vec3(1.0), Type::EmissionStrength },
		{ glm::vec3(0.5), Type::Roughness },
		{ glm::vec3(1.0), Type::Opacity }
	};
};
