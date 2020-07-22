#include "Model.h"

bool Model::Load(std::string filename, std::string texture)
{
	ModelTexture = LoadTexture(texture);
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Fast);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return false;
	}
	aiMesh* mesh = scene->mMeshes[0];
	numVerts = mesh->mNumFaces * 3;
	vertexArray = new float[mesh->mNumFaces * 3 * 3];
	normalArray = new float[mesh->mNumFaces * 3 * 3];
	uvArray = new float[mesh->mNumFaces * 3 * 2];
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace& face = mesh->mFaces[i];

		for (int j = 0; j < 3; j++)
		{
			aiVector3D uv = mesh->mTextureCoords[0][face.mIndices[j]];
			memcpy(uvArray, &uv, sizeof(float) * 2);
			uvArray += 2;

			aiVector3D normal = mesh->mNormals[face.mIndices[j]];
			memcpy(normalArray, &normal, sizeof(float) * 3);
			normalArray += 3;

			aiVector3D pos = mesh->mVertices[face.mIndices[j]];
			memcpy(vertexArray, &pos, sizeof(float) * 3);
			vertexArray += 3;
		}
	}

	uvArray -= mesh->mNumFaces * 3 * 2;
	normalArray -= mesh->mNumFaces * 3 * 3;
	vertexArray -= mesh->mNumFaces * 3 * 3;
	return true;
}

bool Model::Load(std::string filename)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Fast);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return false;
	}
	aiMesh* mesh = scene->mMeshes[0];
	numVerts = mesh->mNumFaces * 3;
	vertexArray = new float[mesh->mNumFaces * 3 * 3];
	normalArray = new float[mesh->mNumFaces * 3 * 3];
	uvArray = new float[mesh->mNumFaces * 3 * 2];

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace& face = mesh->mFaces[i];

		for (int j = 0; j < 3; j++)
		{
			aiVector3D uv = mesh->mTextureCoords[0][face.mIndices[j]];
			memcpy(uvArray, &uv, sizeof(float) * 2);
			uvArray += 2;

			aiVector3D normal = mesh->mNormals[face.mIndices[j]];
			memcpy(normalArray, &normal, sizeof(float) * 3);
			normalArray += 3;

			aiVector3D pos = mesh->mVertices[face.mIndices[j]];
			memcpy(vertexArray, &pos, sizeof(float) * 3);
			vertexArray += 3;
		}
	}

	uvArray -= mesh->mNumFaces * 3 * 2;
	normalArray -= mesh->mNumFaces * 3 * 3;
	vertexArray -= mesh->mNumFaces * 3 * 3;
	return true;
}

void Model::Draw(bool tex)
{
	if (tex) {
		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, ModelTexture);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, 0, vertexArray);
		glNormalPointer(GL_FLOAT, 0, normalArray);

		glTexCoordPointer(2, GL_FLOAT, 0, uvArray);

		glDrawArrays(GL_TRIANGLES, 0, numVerts);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glPopMatrix();
	}
	else {
		glPushMatrix();
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		glVertexPointer(3, GL_FLOAT, 0, vertexArray);
		glNormalPointer(GL_FLOAT, 0, normalArray);

		glDrawArrays(GL_TRIANGLES, 0, numVerts);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glPopMatrix();
	}
}

void Model::Draw(GLuint texture)
{
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, texture);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertexArray);
	glNormalPointer(GL_FLOAT, 0, normalArray);

	glTexCoordPointer(2, GL_FLOAT, 0, uvArray);

	glDrawArrays(GL_TRIANGLES, 0, numVerts);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glPopMatrix();
}
