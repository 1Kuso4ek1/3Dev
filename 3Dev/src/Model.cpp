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

	globalTransform = toglm(scene->mRootNode->mTransformation);
	globalInverseTransform = toglm(scene->mRootNode->mTransformation.Inverse());

	ProcessNode(scene->mRootNode, scene);
	LoadAnimations(scene);
	/*for(auto i : meshes)
		i->GetPose().resize(i->GetBones().size(), glm::mat4(1.0));*/

	Log::Write("Meshes loaded: " + std::to_string(meshes.size()), Log::Type::Info);
	Log::Write("Bones loaded: " + std::to_string(meshes[0]->GetBones().size()), Log::Type::Info);
	Log::Write("Animations loaded: " + std::to_string(anims.size()), Log::Type::Info);
	
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
		for(auto i : meshes[mesh]->GetBones())
			CalculatePose(i, meshes[mesh]);
				
		shader->SetUniform1f("shininess", mat[mesh].GetShininess());
		shader->SetUniform3f("campos", cam.GetPosition().x, cam.GetPosition().y, cam.GetPosition().z);
		shader->SetVectorOfUniformMatrix4("pose", meshes[mesh]->GetPose().size(), meshes[mesh]->GetPose());
		shader->SetUniform1i("bones", !meshes[mesh]->GetBones().empty());

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
		ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene->mRootNode);

	for(int i = 0; i < node->mNumChildren; i++)
		ProcessNode(node->mChildren[i], scene);
}

void Model::ProcessMesh(aiMesh* mesh, aiNode* node)
{
	std::vector<Vertex> data;
	std::vector<GLuint> indices;

	std::unordered_map<std::string, std::pair<int, glm::mat4>> boneMap;
	
	for(int i = 0; i < mesh->mNumVertices; i++)
	{
		glm::vec3 pos = toglm(mesh->mVertices[i]), norm = toglm(mesh->mNormals[i]);
		glm::vec2 uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		data.emplace_back(pos, norm, uv);
	}
	for(int i = 0; i < mesh->mNumFaces; i++)
		for(int j = 0; j < 3; j++) 
			indices.push_back(mesh->mFaces[i].mIndices[j]);
	
	std::string a;
	if(mesh->mNumBones)
	{
		a = std::string(mesh->mBones[0]->mName.C_Str());
		size_t tmp = a.find_first_of('_');
		a.erase(a.begin() + tmp + 1, a.end());
	}

	for(int i = 0; i < mesh->mNumBones; i++)
	{
		auto bone = mesh->mBones[i];
		boneMap[std::string(i == 0 ? "" : a) + std::string(bone->mName.C_Str())] = { i, toglm(bone->mOffsetMatrix) };
		std::cout << bone->mNumWeights << std::endl;
		std::vector<int> nbones;
		nbones.resize(mesh->mNumVertices, 0);
		for(int j = 0; j < bone->mNumWeights; j++)
		{
			int id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;
			nbones[id]++;
			if(nbones[id] <= 4) 
			{
				data[id].ids[nbones[id]] = i;
				data[id].weights[nbones[id]] = bone->mWeights[j].mWeight;
			}
		}
	}

	for (int i = 0; i < data.size(); i++) 
	{
		glm::vec4 & weights = data[i].weights;
		float total = weights.x + weights.y + weights.z + weights.w;
		if (total > 0.0f)
			data[i].weights /= total;
	}
			
	Log::Write("Mesh vertices: " + std::to_string(mesh->mNumVertices), Log::Type::Info);
	Log::Write("Mesh faces: " + std::to_string(mesh->mNumFaces), Log::Type::Info);
	Log::Write("Mesh bones: " + std::to_string(mesh->mNumBones), Log::Type::Info);
	
	std::vector<Bone> bones;
	FindBoneNodes(node, boneMap, bones);
	meshes.emplace_back(std::make_shared<Mesh>(data, indices, mesh->mAABB, bones));
}

void Model::LoadAnimations(const aiScene* scene)
{
	for(int i = 0; i < scene->mNumAnimations; i++)
	{
		Animation tmp;
		auto anim = scene->mAnimations[i];
		float tps = anim->mTicksPerSecond;
		tmp.tps = (tps > 0 ? tps : 1000.0);
		tmp.duration = anim->mDuration * tmp.tps;

		for (int j = 0; j < anim->mNumChannels; j++)
		{
			auto channel = anim->mChannels[j];
			Keyframe kf;
			for (int k = 0; k < channel->mNumPositionKeys; k++)
			{
				kf.posStamps.push_back(channel->mPositionKeys[k].mTime);
				kf.positions.push_back(toglm(channel->mPositionKeys[k].mValue));
			}
			for (int k = 0; k < channel->mNumRotationKeys; k++)
			{
				kf.rotStamps.push_back(channel->mRotationKeys[k].mTime);
				kf.rotations.push_back(toglm(channel->mRotationKeys[k].mValue));
			}
			for (int k = 0; k < channel->mNumScalingKeys; k++)
			{
				kf.scaleStamps.push_back(channel->mScalingKeys[k].mTime);
				kf.scales.push_back(toglm(channel->mScalingKeys[k].mValue));
			}
			tmp.keyframes[channel->mNodeName.C_Str()] = kf;
		}
		anims.push_back(tmp);
	}
}

void Model::FindBoneNodes(aiNode* node, std::unordered_map<std::string, std::pair<int, glm::mat4>> boneMap, std::vector<Bone>& bones)
{
	Bone tmp;
	if(ProcessBone(node, boneMap, tmp))
	{
		bones.push_back(tmp);
		return;
	}

	for(int i = 0; i < node->mNumChildren; i++)
		FindBoneNodes(node->mChildren[i], boneMap, bones);
}

void Model::CalculatePose(Bone& bone, std::shared_ptr<Mesh>& mesh, glm::mat4 parent)
{
	if(anims[0].keyframes.find(bone.name) != anims[0].keyframes.end())
	{
		Keyframe kf = anims[0].keyframes[bone.name];
		if(time.getElapsedTime().asSeconds() * 1000 >= anims[0].duration / 1000) time.restart();
		float dt = fmod(time.getElapsedTime().asSeconds() * 1000, anims[0].duration / 1000);
		auto fraction = TimeFraction(kf.rotStamps, dt);

		glm::vec3 pos = glm::mix(kf.positions[fraction.first - 1], kf.positions[fraction.first], fraction.second);
		glm::quat rot = glm::slerp(kf.rotations[fraction.first - 1], kf.rotations[fraction.first], fraction.second);
		glm::vec3 scale = glm::mix(kf.scales[fraction.first - 1], kf.scales[fraction.first], fraction.second);

		glm::mat4 mpos(1.0), mscale(glm::scale(glm::mat4(1.0), toglm(size))), mrot = glm::toMat4(rot);
		mpos = glm::translate(mpos, pos);
		mscale = glm::scale(mscale, scale);

		glm::mat4 localTransform = mpos * mrot * mscale;
		glm::mat4 globalTransform = parent * localTransform;

		mesh->GetPose()[bone.id] = globalInverseTransform * globalTransform * bone.offset;
		
		for(Bone& child : bone.children)
			CalculatePose(child, mesh, globalTransform);
	}
	else
	{
		glm::mat4 globalTransform = parent * glm::mat4(1.0);

		mesh->GetPose()[bone.id] = globalInverseTransform * globalTransform * bone.offset;
		
		for(Bone& child : bone.children)
			CalculatePose(child, mesh, globalTransform);
	}
}

bool Model::ProcessBone(aiNode* node, std::unordered_map<std::string, std::pair<int, glm::mat4>> bonemap, Bone& out)
{
	if(bonemap.find(node->mName.C_Str()) != bonemap.end())
	{
		out.name = node->mName.C_Str();
		out.id = bonemap[out.name].first;
		out.offset = bonemap[out.name].second;
		for (int i = 0; i < node->mNumChildren; i++)
		{
			Bone child;
			if(ProcessBone(node->mChildren[i], bonemap, child))
				out.children.push_back(child);
		}
		return true;
	}
	return false;
}
