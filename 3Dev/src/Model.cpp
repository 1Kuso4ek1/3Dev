#include "Model.hpp"

Model::Model(Model* model)
{
	autoUpdateAnimation = model->autoUpdateAnimation;
	drawable = model->drawable;
	meshes = model->meshes;
	m = model->m;
	shader = model->shader;
	mat = model->mat;
	anims = model->anims;
	man = model->man;
	globalInverseTransform = model->globalInverseTransform;
	filename = model->filename;
	size = model->size;
	transform = model->transform;

	if(man)
	{
		shapes.resize(meshes.size(), (rp3d::BoxShape*)(nullptr));
        colliders.resize(meshes.size(), nullptr);
		CreateRigidBody();
	    cstype = model->cstype;
        for(int i = 0; i < meshes.size(); i++)
            switch(cstype)
            {
            case CollisionShapeType::Box:
                CreateBoxShape(i); break;
            case CollisionShapeType::Sphere:
                CreateSphereShape(i); break;
            case CollisionShapeType::Capsule:
                CreateCapsuleShape(i); break;
            case CollisionShapeType::Concave:
                CreateConcaveShape(i); break;
            case CollisionShapeType::Convex:
                CreateConvexShape(i); break;
            }
        body->setIsActive(model->body->isActive());
        body->setType(model->body->getType());
	}
}

Model::Model(Shader* shader)
	 : transform({ 0, 0, 0 }, { 0, 0, 0, 1 }), size({ 1, 1, 1 }), shader(shader)
{
	if(shader) this->shader = shader;
}

Model::Model(std::vector<std::shared_ptr<Mesh>> meshes, Shader* shader)
	 : transform({ 0, 0, 0 }, { 0, 0, 0, 1 }), size({ 1, 1, 1 }), meshes(meshes), shader(shader)
{
	if(shader) this->shader = shader;
}

Model::Model(std::string filename, std::vector<Material*> mat, unsigned int flags, PhysicsManager* man, Shader* shader, Matrices* m)
	 : transform({ 0, 0, 0 }, { 0, 0, 0, 1 }), size({ 1, 1, 1 }), mat(mat), filename(filename), man(man)
{
	if(shader) this->shader = shader;
	if(m) this->m = m;
	if(man) body = man->CreateRigidBody(transform);

	Load(filename, flags);
}

void Model::Load(std::string filename, unsigned int flags)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, flags);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		Log::Write("Error while importing '" + filename + "': " + importer.GetErrorString(), Log::Type::Critical);

	globalInverseTransform = toglm(scene->mRootNode->mTransformation.Inverse());

	ProcessNode(scene->mRootNode, scene);
	LoadAnimations(scene);

	if(man)
    {
        shapes.resize(meshes.size(), (rp3d::BoxShape*)(nullptr));
        colliders.resize(meshes.size(), nullptr);
    }

	Log::Write("Meshes loaded: " + std::to_string(meshes.size()), Log::Type::Info);
	Log::Write("Bones loaded: " + std::to_string(meshes[0]->GetBones().size()), Log::Type::Info);
	Log::Write("Animations loaded: " + std::to_string(anims.size()), Log::Type::Info);

	if(mat.empty()) Log::Write("Empty material array passed", Log::Type::Critical);
	if(mat.size() != meshes.size())
	{
		Log::Write("Materials count != meshes count", Log::Type::Warning);
		mat.resize(meshes.size(), mat.back());
	}

	this->filename = filename;
}

void Model::AddMesh(std::shared_ptr<Mesh> mesh)
{
	meshes.push_back(mesh);

	if(man)
    {
        shapes.push_back((rp3d::BoxShape*)(nullptr));
        colliders.push_back(nullptr);
    }
}

void Model::Draw(Camera* cam, std::vector<Light*> lights, bool transparencyPass)
{
    if(!drawable) return;

	m->PushMatrix();

	if(body) transform = body->getTransform();
	rp3d::Vector3 tmp; float a;
	transform.getOrientation().getRotationAngleAxis(a, tmp);

	m->Translate(toglm(transform.getPosition()));
	m->Rotate(a, glm::axis(toglm(transform.getOrientation()))); // Using toglm(tmp) as second argument breaks everything and gives the matrix of nan
	m->Scale(toglm(size));

	if(autoUpdateAnimation)
		UpdateAnimation();

    if(drawable)
    {
        for(int mesh = 0; mesh < meshes.size(); mesh++)
        {
            shader->Bind();
            mat[mesh]->UpdateShader(shader);
            for(int i = 0; i < lights.size(); i++)
                lights[i]->Update(shader, i);

            shader->SetUniform3f("campos", cam->GetPosition().x, cam->GetPosition().y, cam->GetPosition().z);
            shader->SetUniformMatrix4("transformation", meshes[mesh]->GetTransformation());
            shader->SetVectorOfUniformMatrix4("pose", meshes[mesh]->GetPose().size(), meshes[mesh]->GetPose());
            shader->SetUniform1i("bones", !meshes[mesh]->GetBones().empty());
            shader->SetUniform1i("drawTransparency", transparencyPass);

            m->UpdateShader(shader);

            if(transparencyPass)
            {
                glEnable(GL_CULL_FACE);
                glFrontFace(GL_CCW);
            }

            meshes[mesh]->Draw();

            if(transparencyPass)
            {
                glDisable(GL_CULL_FACE);
                glFrontFace(GL_CW);
            }

            mat[mesh]->ResetShader(shader);
        }
    }

	m->PopMatrix();
}

void Model::SetPosition(const rp3d::Vector3& position)
{
	transform.setPosition(position);
	if(body) body->setTransform(transform);
}

void Model::SetOrientation(const rp3d::Quaternion& orientation)
{
	transform.setOrientation(orientation);
	if(body) body->setTransform(transform);
}

void Model::SetSize(const rp3d::Vector3& size)
{
	this->size = size;
	if(body)
	{
		for(int i = 0; i < meshes.size(); i++)
            switch(cstype)
            {
            case CollisionShapeType::Box:
                std::get<0>(shapes[i])->setHalfExtents(std::get<0>(shapes[i])->getHalfExtents() * size);
				break;
            case CollisionShapeType::Sphere:
                std::get<1>(shapes[i])->setRadius(std::get<1>(shapes[i])->getRadius() * size.x);
				break;
            case CollisionShapeType::Capsule:
                std::get<2>(shapes[i])->setRadius(std::get<2>(shapes[i])->getRadius() * size.x);
				std::get<2>(shapes[i])->setHeight(std::get<2>(shapes[i])->getHeight() * size.y);
				break;
            case CollisionShapeType::Concave:
                std::get<3>(shapes[i])->setScale(size);
				break;
            case CollisionShapeType::Convex:
                std::get<4>(shapes[i])->setScale(size);
				break;
            }
	}
}

void Model::SetMaterial(std::vector<Material*> mat)
{
	this->mat = mat;
}

void Model::SetShader(Shader* shader)
{
	this->shader = shader;
}

void Model::SetPhysicsManager(PhysicsManager* man)
{
	this->man = man;
}

void Model::SetIsDrawable(bool drawable)
{
    this->drawable = drawable;
}

void Model::CreateRigidBody()
{
	if(man) body = man->CreateRigidBody(transform);
}

void Model::Move(const rp3d::Vector3& position)
{
	transform.setPosition(transform.getPosition() + position);
	if(body) body->setTransform(transform);
}

void Model::Rotate(const rp3d::Quaternion& orientation)
{
	transform.setOrientation(orientation * transform.getOrientation());
	if(body) body->setTransform(transform);
}

void Model::Expand(const rp3d::Vector3& size)
{
	this->size += size;
}

void Model::CreateBoxShape(int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
    {
        Log::Write("CreateBoxShape(): int mesh is out of meshes array bounds", Log::Type::Error);
        return;
    }
    if(!body)
    {
        Log::Write("CreateBoxShape(): model don't have a RigidBody", Log::Type::Error);
        return;
    }

	if(colliders.size())
		if(colliders[mesh])
			body->removeCollider(colliders[mesh]);

	aiAABB aabb = meshes[mesh]->GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shapes[mesh] = man->CreateBoxShape((rp3d::Vector3(v.x, v.y, v.z) / 2) * size);
	colliders[mesh] = body->addCollider(std::get<0>(shapes[mesh]), tr);
	cstype = CollisionShapeType::Box;
}

void Model::CreateSphereShape(int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
    {
        Log::Write("CreateSphereShape(): int mesh is out of meshes array bounds", Log::Type::Error);
        return;
    }
    if(!body)
    {
        Log::Write("CreateSphereShape(): model don't have a RigidBody", Log::Type::Error);
        return;
    }

    if(colliders.size())
		if(colliders[mesh])
			body->removeCollider(colliders[mesh]);

	aiAABB aabb = meshes[mesh]->GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shapes[mesh] = man->CreateSphereShape((v.y / 2) * size.y);
	colliders[mesh] = body->addCollider(std::get<1>(shapes[mesh]), tr);
	cstype = CollisionShapeType::Sphere;
}

void Model::CreateCapsuleShape(int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
    {
        Log::Write("CreateCapsuleShape(): int mesh is out of meshes array bounds", Log::Type::Error);
        return;
    }
    if(!body)
    {
        Log::Write("CreateCapsuleShape(): model don't have a RigidBody", Log::Type::Error);
        return;
    }

    if(colliders.size())
		if(colliders[mesh])
			body->removeCollider(colliders[mesh]);

	aiAABB aabb = meshes[mesh]->GetAABB();
	auto v = aabb.mMax - aabb.mMin;
	shapes[mesh] = man->CreateCapsuleShape((glm::max(v.x, v.z) / 2) * size.x, (v.y / 2) * size.y);
	colliders[mesh] = body->addCollider(std::get<2>(shapes[mesh]), tr);
	cstype = CollisionShapeType::Capsule;
}

void Model::CreateConcaveShape(int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
    {
        Log::Write("CreateConcaveShape(): int mesh is out of meshes array bounds", Log::Type::Error);
        return;
    }
    if(!body)
    {
        Log::Write("CreateConcaveShape(): model don't have a RigidBody", Log::Type::Error);
        return;
    }

    if(colliders.size())
		if(colliders[mesh])
			body->removeCollider(colliders[mesh]);

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
	colliders[mesh] = body->addCollider(std::get<3>(shapes[mesh]), tr);
	body->setType(rp3d::BodyType::STATIC);
	cstype = CollisionShapeType::Concave;
}

void Model::CreateConvexShape(int mesh, rp3d::Transform tr)
{
    if(mesh >= meshes.size())
    {
        Log::Write("CreateConvexShape(): int mesh is out of meshes array bounds", Log::Type::Error);
        return;
    }
    if(!body)
    {
        Log::Write("CreateConvexShape(): model don't have a RigidBody", Log::Type::Error);
        return;
    }

	if(colliders.size())
		if(colliders[mesh])
			body->removeCollider(colliders[mesh]);

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
	colliders[mesh] = body->addCollider(std::get<4>(shapes[mesh]), tr);
	cstype = CollisionShapeType::Convex;
}

void Model::PlayAnimation(int anim)
{
	if(anim >= anims.size())
    {
        Log::Write("PlayAnimation(): int anim is out of anims array bounds", Log::Type::Error);
        return;
    }

	for(int i = 0; i < GetAnimationsCount(); i++)
		StopAnimation(i);

	anims[anim].state = Animation::State::Playing;
	anims[anim].time.restart();
}

void Model::StopAnimation(int anim)
{
	if(anim >= anims.size())
    {
        Log::Write("StopAnimation(): int anim is out of anims array bounds", Log::Type::Error);
        return;
    }

	anims[anim].state = Animation::State::Stopped;
	for(auto& i : meshes)
		std::fill(i->GetPose().begin(), i->GetPose().end(), glm::mat4(1.0));
}

void Model::PauseAnimation(int anim)
{
	if(anim >= anims.size())
    {
        Log::Write("PauseAnimation(): int anim is out of anims array bounds", Log::Type::Error);
        return;
    }

	anims[anim].state = Animation::State::Paused;
	anims[anim].lastTime = anims[anim].GetTime();
}

void Model::AutoUpdateAnimation(bool update)
{
	autoUpdateAnimation = update;
}

void Model::UpdateAnimation()
{
	for(auto mesh : meshes)
		for(auto i : mesh->GetBones())
			CalculatePose(i, mesh, mesh->GetTransformation());
}

int Model::GetMeshesCount()
{
	return meshes.size();
}

int Model::GetAnimationsCount()
{
	return anims.size();
}

bool Model::IsDrawable()
{
    return drawable;
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

Animation::State Model::GetAnimationState(int anim)
{
	return anims[anim].state;
}

Shader* Model::GetShader()
{
	return shader;
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

std::vector<Material*>& Model::GetMaterial()
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
					i.lastTime = 0;
				}

				float dt = fmod(time, i.duration);
				auto fraction = TimeFraction(kf.rotStamps, dt);

				glm::vec3 pos = glm::mix(kf.positions[fraction.first - 1], kf.positions[fraction.first], fraction.second);
				glm::quat rot = glm::slerp(kf.rotations[fraction.first - 1], kf.rotations[fraction.first], fraction.second);
				glm::vec3 scale = glm::mix(kf.scales[fraction.first - 1], kf.scales[fraction.first], fraction.second);

				glm::mat4 mpos = glm::translate(glm::mat4(1.0), pos),
						  mscale = glm::scale(glm::mat4(1.0), scale),
						  mrot = glm::toMat4(rot);

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

Json::Value Model::Serialize()
{
    Json::Value data;

	auto pos = GetPosition();
	auto orient = GetOrientation();
	auto size = GetSize();

	data["filename"] = GetFilename();

	data["position"]["x"] = pos.x;
	data["position"]["y"] = pos.y;
	data["position"]["z"] = pos.z;

	data["orientation"]["x"] = orient.x;
	data["orientation"]["y"] = orient.y;
	data["orientation"]["z"] = orient.z;
	data["orientation"]["w"] = orient.w;

	data["size"]["x"] = size.x;
	data["size"]["y"] = size.y;
	data["size"]["z"] = size.z;

	data["drawable"] = drawable;

	data["rigidBody"]["active"] = body ? body->isActive() : false;
	data["rigidBody"]["collider"] = (int)cstype;
	data["rigidBody"]["type"] = (int)body->getType();

	return data;
}

void Model::Deserialize(Json::Value data)
{
    rp3d::Vector3 pos, size;
	rp3d::Quaternion orient;

	pos.x = data["position"]["x"].asFloat();
	pos.y = data["position"]["y"].asFloat();
	pos.z = data["position"]["z"].asFloat();

	orient.x = data["orientation"]["x"].asFloat();
	orient.y = data["orientation"]["y"].asFloat();
	orient.z = data["orientation"]["z"].asFloat();
	orient.w = data["orientation"]["w"].asFloat();

	size.x = data["size"]["x"].asFloat();
	size.y = data["size"]["y"].asFloat();
	size.z = data["size"]["z"].asFloat();

	drawable = data["drawable"].asBool();

	if(body)
	{
	    cstype = (CollisionShapeType)data["rigidBody"]["collider"].asInt();
        for(int i = 0; i < meshes.size(); i++)
            switch(cstype)
            {
            case CollisionShapeType::Box:
                CreateBoxShape(i); break;
            case CollisionShapeType::Sphere:
                CreateSphereShape(i); break;
            case CollisionShapeType::Capsule:
                CreateCapsuleShape(i); break;
            case CollisionShapeType::Concave:
                CreateConcaveShape(i); break;
            case CollisionShapeType::Convex:
                CreateConvexShape(i); break;
            }
        body->setIsActive(data["rigidBody"]["active"].asBool());
        body->setType((rp3d::BodyType)data["rigidBody"]["type"].asInt());
	}

	SetPosition(pos);
	SetOrientation(orient);
	SetSize(size);
}
