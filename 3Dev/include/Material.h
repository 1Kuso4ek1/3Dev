#pragma once
#include <3Dev.h>
#include <unordered_map>

/*
 * This class contains ids of various texture maps
 */
class Material
{
public:
	enum class TexType
	{
		Diffuse,
		NormalMap,
		AmbientOcclusion,
		Metalness,
		Emission,
		Roughness,
		Opacity,
		Cubemap
	};

	/*
	 * Basic constructor
	 * @param shininess material shininess
	 * @param textures vector with all texture maps
	 */
	Material(float shininess, std::vector<std::pair<GLuint, Material::TexType>> textures);

	// @param shininess new shininess value
	void SetShininess(float shininess);

	/*
	 * @param texture new texture id
	 * @param type type of a new texture
	 */
	void AddTexture(GLuint texture, TexType type);
	
	// @return shininess of this material
	float GetShininess();

	// @return reference to all texture maps
	std::vector<std::pair<GLuint, TexType>>& GetTextures();

private:
	float shininess;

	std::vector<std::pair<GLuint, TexType>> textures;
};
