#include "Model.h"

Model::Model(std::string filename, std::vector<Material> mat, Shader* shader, Matrices* m, PhysicsManager* man, unsigned int flags, rp3d::Vector3 position, rp3d::Quaternion orientation, rp3d::Vector3 size) : transform(position, orientation), size(size), mat(mat), shader(shader), filename(filename), m(m), man(man)
{
	if(man != nullptr) body = man->CreateRigidBody(transform);

	Load(filename, flags);
}

void Model::Load(std::string filename, unsigned int flags)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, flags);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		Log::Write("Error while importing '" + filename + "': " + importer.GetErrorString(), Log::Type::Critical);
	ProcessNode(scene->mRootNode, scene);
	Log::Write("Meshes loaded: " + std::to_string(meshes.size()), Log::Type::Info);
	if(mat.empty()) Log::Write("Empty material array passed", Log::Type::Critical);
	if(mat.size() != meshes.size())
	{
		Log::Write("Materials count != meshes count", Log::Type::Warning);
		mat.resize(meshes.size(), mat.back());
	}
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

	for(int mesh = 0; mesh < meshes.size(); mesh++)
	{
		shader->Bind();
		mat[mesh].UpdateShader(shader);
		for(int i = 0; i < lights.size(); i++)
			lights[i].Update(shader, i);
		shader->SetUniform1f("shininess", mat[mesh].GetShininess());
		shader->SetUniform3f("campos", cam.GetPosition().x, cam.GetPosition().y, cam.GetPosition().z);
		m->UpdateShader(shader);

		meshes[mesh]->Draw();
		
		mat[mesh].ResetShader(shader);
	}
	
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

void Model::SetMaterial(std::vector<Material> mat)
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
	aiAABB aabb = meshes[0]->GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shape = man->CreateBoxShape((rp3d::Vector3(v.x, v.y, v.z) / 2) * size);
	body->addCollider(shape, rp3d::Transform::identity());
}

void Model::CreateSphereShape()
{
	aiAABB aabb = meshes[0]->GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shape = man->CreateSphereShape((v.y / 2) * size.y);
	body->addCollider(shape, rp3d::Transform::identity());
}

void Model::CreateCapsuleShape()
{
	aiAABB aabb = meshes[0]->GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shape = man->CreateCapsuleShape((glm::max(v.x, v.z) / 2) * size.x, v.y / 2);
	body->addCollider(shape, rp3d::Transform::identity());
}

void Model::CreateConcaveShape()
{
	triangles = new rp3d::TriangleVertexArray(
	meshes[0]->GetData().size(), &meshes[0]->GetData()[0], sizeof(Vertex),
	&meshes[0]->GetData()[0].normal.x, sizeof(Vertex),
	meshes[0]->GetIndices().size() / 3, &meshes[0]->GetIndices()[0], 3 * sizeof(GLuint),
	rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
	rp3d::TriangleVertexArray::NormalDataType::NORMAL_FLOAT_TYPE,
	rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

	mesh = man->CreateTriangleMesh();
	mesh->addSubpart(triangles);
	shape = man->CreateConcaveMeshShape(mesh, size);
	body->addCollider(shape, rp3d::Transform::identity());
	body->setType(rp3d::BodyType::STATIC);
}

void Model::CreateConvexShape()
{
	faces = new rp3d::PolygonVertexArray::PolygonFace[meshes[0]->GetIndices().size() / 3];
	for (int i = 0; i < meshes[0]->GetIndices().size() / 3; i++)
	{
		faces[i].indexBase = i * 3;
		faces[i].nbVertices = 3;
	}
	polygons = new rp3d::PolygonVertexArray(
	meshes[0]->GetData().size(), &meshes[0]->GetData()[0], sizeof(Vertex),
	&meshes[0]->GetIndices()[0], 3 * sizeof(GLuint), meshes[0]->GetIndices().size() / 3, faces,
	rp3d::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
	rp3d::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

	pmesh = man->CreatePolyhedronMesh(polygons);
	shape = man->CreateConvexMeshShape(pmesh, size);
	body->addCollider(shape, rp3d::Transform::identity());
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

std::vector<Material> Model::GetMaterial()
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
			
	Log::Write("Mesh vertices: " + std::to_string(mesh->mNumVertices), Log::Type::Info);
	Log::Write("Mesh faces: " + std::to_string(mesh->mNumFaces), Log::Type::Info);
	
	meshes.emplace_back(std::make_shared<Mesh>(data, indices, mesh->mAABB));
}
