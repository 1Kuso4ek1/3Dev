#include "Model.h"

Model::Model(std::string filename, Material* mat, Shader* shader, Matrices* m, PhysicsManager* man, unsigned int flags, rp3d::Vector3 position, rp3d::Quaternion orientation, rp3d::Vector3 size) : transform(position, orientation), size(size), mat(mat), shader(shader), filename(filename), m(m), man(man)
{
	if(man != nullptr)
	{
		body = man->CreateRigidBody(transform);
	}

	Load(filename, flags);
}

void Model::Load(std::string filename, unsigned int flags)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, flags);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		Log::Write("Error while importing '" + filename + "': " + importer.GetErrorString(), Log::Type::Critical);
	ProcessNode(scene->mRootNode, scene);
}

void Model::Draw(Camera& cam, std::vector<Light> lights)
{
	m->PushMatrix();

	if(body != nullptr) transform = body->getTransform();
	rp3d::Vector3 tmp; float a;
	transform.getOrientation().getRotationAngleAxis(a, tmp);
	
	m->Translate(toglm(transform.getPosition()));
	m->Rotate(a, glm::axis(toglm(transform.getOrientation()))); // Using toglm(tmp) as second argument breaks everything and gives the matrix of nan
	m->Scale(toglm(size));

	auto textures = mat->GetTextures();
	int diff = 0, norm = 0, ao = 0, emiss = 0, metal = 0, rough = 0, opac = 0;
	bool cubemap = false;
	shader->Bind();
	for(int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		if(textures[i].second != Material::TexType::Cubemap) glBindTexture(GL_TEXTURE_2D, textures[i].first);
		else glBindTexture(GL_TEXTURE_CUBE_MAP, textures[i].first);

		switch(textures[i].second)
		{
		case Material::TexType::Diffuse:
			shader->SetUniform1i("ndiff", 1);
			shader->SetUniform1i(std::string("diff")/* + std::to_string(diff)*/, i);
			diff++;
			break;
		case Material::TexType::NormalMap:
			shader->SetUniform1i("nnormalmap", 1);
			shader->SetUniform1i(std::string("normalmap")/* + std::to_string(norm)*/, i);
			norm++;
			break;
		case Material::TexType::AmbientOcclusion:
			shader->SetUniform1i("nao", 1);
			shader->SetUniform1i(std::string("ao")/* + std::to_string(ao)*/, i);
			ao++;
			break;
		case Material::TexType::Metalness:
			shader->SetUniform1i("nmetalness", 1);
			shader->SetUniform1i(std::string("metalness")/* + std::to_string(metal)*/, i);
			metal++;
			break;
		case Material::TexType::Emission:
			shader->SetUniform1i("nemission", 1);
			shader->SetUniform1i(std::string("emission")/* + std::to_string(emiss)*/, i);
			emiss++;
			break;
		case Material::TexType::Roughness:
			shader->SetUniform1i("nroughness", 1);
			shader->SetUniform1i(std::string("roughness")/* + std::to_string(rough) + "]"*/, i);
			rough++;
			break;
		case Material::TexType::Opacity:
			shader->SetUniform1i("nopacity", 1);
			shader->SetUniform1i(std::string("opacity")/* + std::to_string(rough) + "]"*/, i);
			opac++;
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
	for(int i = 0; i < lights.size(); i++)
		lights[i].Update(shader, i);
	shader->SetUniform1f("shininess", mat->GetShininess());
	shader->SetUniform3f("campos", cam.GetPosition().x, cam.GetPosition().y, cam.GetPosition().z);
	m->UpdateShader(shader);

	for(auto& i : meshes) i.Draw();
	
	m->PopMatrix();
}

void Model::SetPosition(rp3d::Vector3 position)
{
	transform.setPosition(position);
	if(body != nullptr) body->setTransform(transform);
}

void Model::SetOrientation(rp3d::Quaternion orientation)
{
	transform.setOrientation(orientation);
	if(body != nullptr) body->setTransform(transform);
}

void Model::SetSize(rp3d::Vector3 size)
{
	this->size = size;
}

void Model::SetMaterial(Material* mat)
{
	this->mat = mat;
}

void Model::AddPosition(rp3d::Vector3 position) 
{
	transform.setPosition(transform.getPosition() + position);
	if(body != nullptr) body->setTransform(transform);
}

void Model::AddRotation(rp3d::Quaternion orientation) 
{
	transform.setOrientation(orientation * transform.getOrientation());
	if(body != nullptr) body->setTransform(transform);
}

void Model::AddSize(rp3d::Vector3 size) 
{
	this->size += size;
}

void Model::CreateBoxShape()
{
	aiAABB aabb = meshes[0].GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shape = man->CreateBoxShape((rp3d::Vector3(v.x, v.y, v.z) / 2) * size);
	body->addCollider(shape, rp3d::Transform::identity());
}

void Model::CreateSphereShape()
{
	aiAABB aabb = meshes[0].GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shape = man->CreateSphereShape((v.y / 2) * size.y);
	body->addCollider(shape, rp3d::Transform::identity());
}

void Model::CreateCapsuleShape()
{
	aiAABB aabb = meshes[0].GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shape = man->CreateCapsuleShape((glm::max(v.x, v.z) / 2) * size.x, v.y / 2);
	body->addCollider(shape, rp3d::Transform::identity());
}

void Model::CreateConcaveShape()
{

}

void Model::CreateConvexShape()
{

}

rp3d::Vector3 Model::GetPosition() 
{
	return transform.getPosition();
}

rp3d::Quaternion Model::GetOrientation() 
{
	return transform.getOrientation();
}

rp3d::Vector3 Model::GetSize() 
{
	return size;
}

std::string Model::GetFilename() 
{
	return filename;
}

Material* Model::GetMaterial()
{
	return mat;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for(int i = 0; i < node->mNumMeshes; i++)
		ProcessMesh(scene->mMeshes[node->mMeshes[i]]);

	for(int i = 0; i < node->mNumChildren; i++)
		ProcessNode(node->mChildren[i], scene);
}

void Model::ProcessMesh(aiMesh* mesh)
{
	std::vector<Vertex> data;
	std::vector<GLuint> indices;
	
	for(int i = 0; i < mesh->mNumVertices; i++)
	{
		glm::vec3 pos = toglm(mesh->mVertices[i]), norm = toglm(mesh->mNormals[i]);
		glm::vec2 uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		data.emplace_back(pos, norm, uv);
	}

	for(int i = 0; i < mesh->mNumFaces; i++)
		for(int j = 0; j < 3; j++) 
			indices.push_back(mesh->mFaces[i].mIndices[j]);
			
	meshes.emplace_back(data, indices, mesh->mAABB);
}
