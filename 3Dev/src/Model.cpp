#include "Model.hpp"

Model::Model(Shader* shader)
			: transform({ 0, 0, 0 }, { 0, 0, 0, 1 }), size({ 1, 1, 1 }), shader(shader) {}

Model::Model(std::vector<std::shared_ptr<Mesh>> meshes, Shader* shader)
			: transform({ 0, 0, 0 }, { 0, 0, 0, 1 }), size({ 1, 1, 1 }), meshes(meshes), shader(shader) {}

Model::Model(std::string filename, std::vector<Material> mat, Shader* shader, Matrices* m, PhysicsManager* man, unsigned int flags)
			: transform({ 0, 0, 0 }, { 0, 0, 0, 1 }), size({ 1, 1, 1 }), mat(mat), shader(shader), filename(filename), m(m), man(man)
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
	if(man != nullptr)
		shapes.resize(meshes.size(), nullptr);

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

void Model::Draw(Camera* cam, std::vector<Light*> lights)
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
			lights[i]->Update(shader, i);
		for(auto i : meshes[mesh]->GetBones())
			CalculatePose(i, meshes[mesh], meshes[mesh]->GetTransformation());
				
		shader->SetUniform3f("campos", cam->GetPosition().x, cam->GetPosition().y, cam->GetPosition().z);
		shader->SetUniformMatrix4("transformation", meshes[mesh]->GetTransformation());
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

void Model::SetShader(Shader* shader)
{
	this->shader = shader;
}

void Model::Move(rp3d::Vector3 position) 
{
	transform.setPosition(transform.getPosition() + position);
	if(body != nullptr) body->setTransform(transform);
}

void Model::Rotate(rp3d::Quaternion orientation) 
{
	transform.setOrientation(orientation * transform.getOrientation());
	if(body != nullptr) body->setTransform(transform);
}

void Model::Expand(rp3d::Vector3 size) 
{
	this->size += size;
}

void Model::CreateBoxShape(int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
		Log::Write("int mesh is out of meshes array bounds!", Log::Type::Critical);

	aiAABB aabb = meshes[mesh]->GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shapes[mesh] = man->CreateBoxShape((rp3d::Vector3(v.x, v.y, v.z) / 2) * size);
	body->addCollider(shapes[mesh], tr);
}

void Model::CreateSphereShape(int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
		Log::Write("int mesh is out of meshes array bounds!", Log::Type::Critical);

	aiAABB aabb = meshes[mesh]->GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shapes[mesh] = man->CreateSphereShape((v.y / 2) * size.y);
	body->addCollider(shapes[mesh], tr);
}

void Model::CreateCapsuleShape(int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
		Log::Write("int mesh is out of meshes array bounds!", Log::Type::Critical);

	aiAABB aabb = meshes[mesh]->GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shapes[mesh] = man->CreateCapsuleShape((glm::max(v.x, v.z) / 2) * size.x, v.y / 2);
	body->addCollider(shapes[mesh], tr);
}

void Model::CreateConcaveShape(int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
		Log::Write("int mesh is out of meshes array bounds!", Log::Type::Critical);

	triangles = new rp3d::TriangleVertexArray(
	meshes[mesh]->GetData().size(), &meshes[mesh]->GetData()[0], sizeof(Vertex),
	&meshes[mesh]->GetData()[0].normal.x, sizeof(Vertex),
	meshes[mesh]->GetIndices().size() / 3, &meshes[mesh]->GetIndices()[0], 3 * sizeof(GLuint),
	rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
	rp3d::TriangleVertexArray::NormalDataType::NORMAL_FLOAT_TYPE,
	rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

	tmesh = man->CreateTriangleMesh();
	tmesh->addSubpart(triangles);
	shapes[mesh] = man->CreateConcaveMeshShape(tmesh, size);
	body->addCollider(shapes[mesh], tr);
	body->setType(rp3d::BodyType::STATIC);
}

void Model::CreateConvexShape(int mesh, rp3d::Transform tr)
{
	faces = new rp3d::PolygonVertexArray::PolygonFace[meshes[mesh]->GetIndices().size() / 3];
	for (int i = 0; i < meshes[mesh]->GetIndices().size() / 3; i++)
	{
		faces[i].indexBase = i * 3;
		faces[i].nbVertices = 3;
	}
	polygons = new rp3d::PolygonVertexArray(
	meshes[mesh]->GetData().size(), &meshes[mesh]->GetData()[0], sizeof(Vertex),
	&meshes[mesh]->GetIndices()[0], 3 * sizeof(GLuint), meshes[mesh]->GetIndices().size() / 3, faces,
	rp3d::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
	rp3d::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

	pmesh = man->CreatePolyhedronMesh(polygons);
	shapes[mesh] = man->CreateConvexMeshShape(pmesh, size);
	body->addCollider(shapes[mesh], tr);
}

void Model::PlayAnimation(int anim)
{
	if(anim >= anims.size())
		Log::Write("int anim is out of anims array bounds!", Log::Type::Critical);

	anims[anim].state = Animation::State::Playing;
	anims[anim].time.restart();
}

void Model::StopAnimation(int anim)
{
	if(anim >= anims.size())
		Log::Write("int anim is out of anims array bounds!", Log::Type::Critical);

	anims[anim].state = Animation::State::Stopped;
	for(auto& i : meshes)
		for(auto& j : i->GetPose())
			j = glm::mat4(1.0);
}

void Model::PauseAnimation(int anim)
{
	if(anim >= anims.size())
		Log::Write("int anim is out of anims array bounds!", Log::Type::Critical);

	anims[anim].state = Animation::State::Paused;
	anims[anim].lastTime = anims[anim].GetTime();
}

int Model::GetMeshesCount()
{
	return meshes.size();
}

int Model::GetAnimationsCount()
{
	return anims.size();
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

rp3d::RigidBody* Model::GetRigidBody()
{
	return body;
}

std::vector<Bone>& Model::GetBones(int mesh)
{
	if(mesh >= meshes.size())
		Log::Write("int mesh is out of meshes array bounds!", Log::Type::Critical);
	
	return meshes[mesh]->GetBones();
}

std::vector<glm::mat4>& Model::GetPose(int mesh)
{
	if(mesh >= meshes.size())
		Log::Write("int mesh is out of meshes array bounds!", Log::Type::Critical);
	
	return meshes[mesh]->GetPose();
}

std::string Model::GetFilename() 
{
	return filename;
}

std::vector<Material>& Model::GetMaterial()
{
	return mat;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for(int i = 0; i < node->mNumMeshes; i++)
		ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene->mRootNode, node);

	for(int i = 0; i < node->mNumChildren; i++)
		ProcessNode(node->mChildren[i], scene);
}

void Model::ProcessMesh(aiMesh* mesh, aiNode* node, aiNode* mnode)
{
	std::vector<Vertex> data;
	std::vector<GLuint> indices;

	glm::mat4 tr = globalInverseTransform * toglm(mnode->mTransformation);
	for(int i = 0; i < 4; i++)
		tr[i] = glm::normalize(tr[i]);

	std::unordered_map<std::string, std::pair<int, glm::mat4>> boneMap;
	
	for(int i = 0; i < mesh->mNumVertices; i++)
	{
		glm::vec3 pos = toglm(mesh->mVertices[i]),
				  norm = toglm(mesh->mNormals[i]);
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
		std::string tmp;
		if(std::string(bone->mName.C_Str()).find(a) == std::string::npos)
			tmp = a + bone->mName.C_Str();
		else tmp = bone->mName.C_Str();

		boneMap[tmp] = { i, toglm(bone->mOffsetMatrix) };
		std::vector<int> nbones;
		nbones.resize(mesh->mNumVertices, 0);
		for(int j = 0; j < bone->mNumWeights; j++)
		{
			int id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;
			if(nbones[id] < 4) 
			{
				data[id].ids[nbones[id]] = i;
				data[id].weights[nbones[id]] = weight;
			}
			nbones[id]++;
		}
	}

	for (int i = 0; i < data.size(); i++) 
	{
		glm::vec4& weights = data[i].weights;
		float total = weights.x + weights.y + weights.z + weights.w;
		if(total > 0.0f)
			data[i].weights /= total;
	}
			
	Log::Write("Mesh vertices: " + std::to_string(mesh->mNumVertices), Log::Type::Info);
	Log::Write("Mesh faces: " + std::to_string(mesh->mNumFaces), Log::Type::Info);
	Log::Write("Mesh bones: " + std::to_string(mesh->mNumBones), Log::Type::Info);
	
	std::vector<Bone> bones;
	FindBoneNodes(node, boneMap, bones);
	meshes.emplace_back(std::make_shared<Mesh>(data, indices, mesh->mAABB, bones, tr));
}

void Model::LoadAnimations(const aiScene* scene)
{
	for(int i = 0; i < scene->mNumAnimations; i++)
	{
		Animation tmp;
		auto anim = scene->mAnimations[i];
		float tps = anim->mTicksPerSecond;
		tmp.tps = (tps > 0 ? tps : 1000.0);
		tmp.duration = anim->mDuration;

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
	for(auto i : anims)
		if(i.state == Animation::State::Playing || i.state == Animation::State::Paused)
			if(i.keyframes.find(bone.name) != i.keyframes.end())
			{
				Keyframe kf = i.keyframes[bone.name];

				float time = 0;
				if(i.state == Animation::State::Playing)
				{
					time = i.GetTime();
					if(i.lastTime != 0)
						time += i.lastTime;
				}
				else time = i.lastTime;

				if(i.state == Animation::State::Playing && time >= i.duration)
				{
					time = i.time.restart().asSeconds() * i.tps;
					if(i.lastTime != 0) i.lastTime = 0;
				}

				float dt = fmod(time, i.duration);
				auto fraction = TimeFraction(kf.rotStamps, dt);

				glm::vec3 pos = glm::mix(kf.positions[fraction.first - 1], kf.positions[fraction.first], fraction.second);
				glm::quat rot = glm::slerp(kf.rotations[fraction.first - 1], kf.rotations[fraction.first], fraction.second);
				glm::vec3 scale = glm::mix(kf.scales[fraction.first - 1], kf.scales[fraction.first], fraction.second);

				glm::mat4 mpos(1.0), mscale(1.0), mrot = glm::toMat4(rot);
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
				glm::mat4 globalTransform = parent;
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
