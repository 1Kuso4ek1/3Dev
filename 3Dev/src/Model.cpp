#include "Model.hpp"

Model::Model(bool defaultCubeMesh)
{
	if(defaultCubeMesh)
	{
		meshes.emplace_back(std::make_shared<Mesh>());
		meshes.back()->CreateCube();
		loaded = true;
	}
}

Model::~Model()
{
	if(man && body)
		man->GetWorld()->destroyRigidBody(body);
}

Model::Model(Model* model)
{
	drawable = model->drawable;
	m = model->m;
	shader = model->shader;
	mat = model->mat;
	man = model->man;
	filename = model->filename;
	size = model->size;
	transform = model->transform;
	shadowBias = model->shadowBias;
	loaded = model->loaded;

	if(!filename.empty() && loaded)
		Load(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
	else meshes = model->meshes;

	for(int i = 0; i < bones.size(); i++)
	{
		bones[i]->SetTransform(model->bones[i]->GetTransform());
		bones[i]->SavePoseAsIdle();
	}

	for(int i = 0; i < bonesChildren.size(); i++)
	{
		bonesChildren[i]->SetTransform(model->bonesChildren[i]->GetTransform());
		bonesChildren[i]->SavePoseAsIdle();
	}

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
			default: break;
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
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 1);
	const aiScene* scene = importer.ReadFile(filename, flags | aiProcess_LimitBoneWeights);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		Log::Write("Error while importing '" + filename + "': " + importer.GetErrorString(), Log::Type::Critical);

	globalInverseTransform = toglm(scene->mRootNode->mTransformation.Inverse());

	meshes.clear();
	anims.clear();

	LoadAnimations(scene);
	ProcessNode(scene->mRootNode, scene);
	FindBoneNodes(scene->mRootNode, bones);

	pose.resize(64);

	if(man)
    {
        shapes.resize(meshes.size(), (rp3d::BoxShape*)(nullptr));
        colliders.resize(meshes.size(), nullptr);
    }

	Log::Write("Model " + filename + " loaded:\n" + 
			   "        Meshes: " + std::to_string(meshes.size()) + '\n' +
			   "        Bones: " + std::to_string(bones.size() + bonesChildren.size()) + '\n' +
			   "        Animations: " + std::to_string(anims.size()), Log::Type::Info);

	if(mat.empty()) Log::Write("Empty material array passed", Log::Type::Critical);
	if(mat.size() != meshes.size())
		mat.resize(meshes.size(), mat.back());

	for(auto& i : mat)
		i->LoadTextures();

	this->filename = filename;
	loaded = true;
}

void Model::Load()
{
	Load(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
	if(!data.empty())
		Deserialize(data);
}

void Model::Unload(bool unloadTextures)
{
	meshes.clear();
	shapes.clear();

	if(body)
		for(auto i : colliders)
			body->removeCollider(i);

	colliders.clear();

	if(unloadTextures)
		for(auto& i : mat)
			i->UnloadTextures();

	loaded = false;
}

void Model::Draw(Node* cam, std::vector<Node*> lights, bool transparencyPass)
{
	if(!loaded) return;

	m->PushMatrix();

	auto tr = Node::GetFinalTransform(this);

	if(body)
	{
		if(tr == rp3d::Transform::identity())
			transform = body->getTransform();
		else body->setTransform(tr * transform);
	}
	rp3d::Vector3 tmp; float a;
	(tr.getOrientation() * transform.getOrientation()).getRotationAngleAxis(a, tmp);

	m->Translate(toglm((tr * transform).getPosition()));
	m->Rotate(a, glm::axis(toglm((tr * transform).getOrientation()))); // Using toglm(tmp) as second argument breaks everything and gives the matrix of nan
	m->Scale(toglm(size));

	for(auto& i : bones)
		CalculatePose(i.get());

    if(drawable)
    {
		auto shader = (tempShader ? tempShader : this->shader);

        for(unsigned int mesh = 0; mesh < meshes.size(); mesh++)
        {
            shader->Bind();
            mat[mesh]->UpdateShader(shader);
			if(transparencyPass)
			{
				for(int i = 0; i < 64; i++)
					if(i >= lights.size())
						shader->SetUniform1i("lights[" + std::to_string(i) + "].isactive", 0);
					else dynamic_cast<Light*>(lights[i])->Update(shader, i);
			}

			auto camPos = dynamic_cast<Camera*>(cam)->GetPosition(true);
            shader->SetUniform3f("campos", camPos.x, camPos.y, camPos.z);
            shader->SetUniformMatrix4("transformation", glm::mat4(1.0));
			if(!pose.empty())
            	shader->SetVectorOfUniformMatrix4("pose", pose.size(), pose);
            shader->SetUniform1i("bones", !bones.empty());
            shader->SetUniform1i("drawTransparency", transparencyPass);
			shader->SetUniform1f("shadowBias", shadowBias);

            m->UpdateShader(shader);

            meshes[mesh]->Draw();

            mat[mesh]->ResetShader(shader);
        }
    }

	m->PopMatrix();

	for(auto i : children)
		i->Draw(cam, lights, transparencyPass);

	tempShader = nullptr;
}

void Model::DrawSkybox()
{
	if(!drawable || !mat[0]->Contains(Material::Type::Cubemap)) return;

	auto shader = Renderer::GetInstance()->GetShader(Renderer::ShaderType::Skybox);

	m->PushMatrix();

	shader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, std::get<1>(mat[0]->GetParameter(Material::Type::Cubemap)));

	shader->SetUniform1i("cubemap", 0);
	m->UpdateShader(shader);

	meshes[0]->Draw();

	m->PopMatrix();
}

void Model::SetTransform(const rp3d::Transform& transform)
{
	this->transform = transform;
	if(body) body->setTransform(transform);
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
                //std::get<0>(shapes[i])->setHalfExtents(std::get<0>(shapes[i])->getHalfExtents() * size);
				CreateBoxShape();
				break;
            case CollisionShapeType::Sphere:
                //std::get<1>(shapes[i])->setRadius(std::get<1>(shapes[i])->getRadius() * size.x);
				CreateSphereShape();
				break;
            case CollisionShapeType::Capsule:
                /*std::get<2>(shapes[i])->setRadius(std::get<2>(shapes[i])->getRadius() * size.x);
				std::get<2>(shapes[i])->setHeight(std::get<2>(shapes[i])->getHeight() * size.y);*/
				CreateCapsuleShape();
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

void Model::SetMaterial(std::vector<Material*> mat, bool loadTextures)
{
	if(loadTextures)
		for(auto i : mat)
			i->LoadTextures();
	this->mat = mat;
}

void Model::SetMaterialSlot(Material* mat, unsigned int slot)
{
	if(slot >= this->mat.size())
	{
		Log::Write("SetMaterial(): unsigned int slot is out of mat array bounds", Log::Type::Error);
        return;
	}

	mat->LoadTextures();
	this->mat[slot] = mat;
}

void Model::SetShader(Shader* shader, bool temp)
{
	if(temp) tempShader = shader;
	else this->shader = shader;
}

void Model::SetPhysicsManager(PhysicsManager* man)
{
	this->man = man;
	shapes.resize(meshes.size(), (rp3d::BoxShape*)(nullptr));
    colliders.resize(meshes.size(), nullptr);
}

void Model::SetFilename(const std::string& filename)
{
	this->filename = filename;
}

void Model::SetIsDrawable(bool drawable)
{
    this->drawable = drawable;
}

void Model::SetIsLoadingImmediatelly(bool imm)
{
	immLoad = imm;
}

void Model::SetShadowBias(float bias)
{
	shadowBias = bias;
}

void Model::AddChild(Node* child)
{
	for(auto i : bones)
		if(i.get() == child)
		{
			auto it = std::find(children.begin(), children.end(), child);
			if(it != children.end())
			{
				(*it)->SetParent(nullptr);
				children.erase(it);
				return;
			}
			child->SetParent(this);
			children.push_back(child);
			return;
		}

	auto it = std::find(children.begin(), children.end(), child);
    if(it != children.end())
	{
		if(child->GetRigidBody() && body)
		{
			child->GetRigidBody()->setIsActive(true);
			for(auto& i : childrenColliders[child])
				body->removeCollider(i);
			childrenColliders.erase(childrenColliders.find(child));
		}
        children.erase(it);
	}
    else
	{
		if(child->GetRigidBody() && body)
		{
			child->GetRigidBody()->setIsActive(false);
			for(int i = 0; i < child->GetRigidBody()->getNbColliders(); i++)
				childrenColliders[child].push_back(body->addCollider(child->GetRigidBody()->getCollider(i)->getCollisionShape(), child->GetTransform()));
		}
		children.push_back(child);
	}
}

void Model::DefaultPose()
{
	for(auto& i : bones)
		i->SetTransform(i->GetIdle());
	for(auto& i : bonesChildren)
		i->SetTransform(i->GetIdle());
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
	SetSize(this->size + size);
}

void Model::CreateBoxShape(unsigned int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
    {
        Log::Write("CreateBoxShape(): unsigned int mesh is out of meshes array bounds", Log::Type::Error);
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

void Model::CreateSphereShape(unsigned int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
    {
        Log::Write("CreateSphereShape(): unsigned int mesh is out of meshes array bounds", Log::Type::Error);
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

void Model::CreateCapsuleShape(unsigned int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
    {
        Log::Write("CreateCapsuleShape(): unsigned int mesh is out of meshes array bounds", Log::Type::Error);
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

void Model::CreateConcaveShape(unsigned int mesh, rp3d::Transform tr)
{
	if(mesh >= meshes.size())
    {
        Log::Write("CreateConcaveShape(): unsigned int mesh is out of meshes array bounds", Log::Type::Error);
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

void Model::CreateConvexShape(unsigned int mesh, rp3d::Transform tr)
{
    if(mesh >= meshes.size())
    {
        Log::Write("CreateConvexShape(): unsigned int mesh is out of meshes array bounds", Log::Type::Error);
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

int Model::GetMeshesCount()
{
	return meshes.size();
}

bool Model::IsLoaded()
{
	return loaded;
}

bool Model::IsDrawable()
{
    return drawable;
}

bool Model::IsLoadingImmediatelly()
{
	return immLoad;
}

rp3d::Transform Model::GetTransform()
{
	return transform;
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

Shader* Model::GetShader()
{
	return shader;
}

rp3d::RigidBody* Model::GetRigidBody()
{
	return body;
}

std::vector<std::shared_ptr<Animation>> Model::GetAnimations()
{
	return anims;
}

std::vector<std::shared_ptr<Bone>> Model::GetBones()
{
	return bones;
}

std::vector<glm::mat4>& Model::GetPose()
{
	return pose;
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

	glm::mat4 tr(1.0);
	if(mnode->mParent)
		tr = toglm(mnode->mParent->mTransformation) * toglm(mnode->mTransformation);
	for(int i = 0; i < 4; i++)
		tr[i] = glm::normalize(tr[i]);

	for(int i = 0; i < mesh->mNumVertices; i++)
	{
		glm::vec3 pos = toglm(mesh->mVertices[i]),
				  norm = toglm(mesh->mNormals[i]);
		glm::vec2 uv(0.0);
		if(mesh->mTextureCoords[0])
			uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		data.emplace_back(pos, norm, uv);
	}
	for(int i = 0; i < mesh->mNumFaces; i++)
		for(int j = 0; j < 3; j++)
			indices.push_back(mesh->mFaces[i].mIndices[j]);

	std::string a;

	for(int i = 0; i < mesh->mNumBones; i++)
	{
		auto bone = mesh->mBones[i];
		std::string tmp;
		if(std::string(bone->mName.C_Str()).find(a) == std::string::npos)
			tmp = a + bone->mName.C_Str();
		else tmp = bone->mName.C_Str();

		bonemap[tmp] = { i, toglm(bone->mOffsetMatrix) };
		std::vector<int> nbones;
		nbones.resize(mesh->mNumVertices, 0);
		for(int j = 0; j < bone->mNumWeights; j++)
		{
			int id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;
			data[id].ids[nbones[id]] = i;
			data[id].weights[nbones[id]] = weight;
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

	meshes.emplace_back(std::make_shared<Mesh>(data, indices, mesh->mAABB, tr));
}

void Model::LoadAnimations(const aiScene* scene)
{
	for(int i = 0; i < scene->mNumAnimations; i++)
	{
		auto anim = scene->mAnimations[i];
		auto tmp = std::make_shared<Animation>(anim->mName.C_Str());
		float tps = anim->mTicksPerSecond;
		tmp->SetTPS(tps > 0 ? tps : 30.0);
		tmp->SetDuration(anim->mDuration);

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
			tmp->AddKeyframe(channel->mNodeName.C_Str(), kf);
		}
		anims.push_back(tmp);
	}
}

void Model::FindBoneNodes(aiNode* node, std::vector<std::shared_ptr<Bone>>& bones)
{
	std::shared_ptr<Bone> tmp = nullptr;
	if(ProcessBone(node, tmp))
	{
		if(tmp)
		{
			bones.push_back(tmp);
			this->AddChild(tmp.get());
		}
		return;
	}

	for(int i = 0; i < node->mNumChildren; i++)
		FindBoneNodes(node->mChildren[i], bones);
}

void Model::CalculatePose(Bone* bone)
{
	auto it = std::find_if(bones.begin(), bones.end(), [&](auto b)
						  { return b.get() == bone; });
	auto it1 = std::find_if(bonesChildren.begin(), bonesChildren.end(), [&](auto b)
						  { return b.get() == bone; });
	if(it == bones.end() && it1 == bonesChildren.end())
		return;
		
	rp3d::Transform tmp = transform;
	transform = rp3d::Transform::identity();

	auto finalTransform = Node::GetFinalTransform(bone) * bone->GetTransform();

	glm::mat4 finaltr(1.0);
	finaltr = glm::translate(finaltr, toglm(finalTransform.getPosition()));
	finaltr = finaltr * glm::toMat4(toglm(finalTransform.getOrientation()));
	finaltr = glm::scale(finaltr, toglm(bone->GetSize()));

	pose[bone->GetID()] = finaltr * bone->GetOffset();

	for(auto child : bone->GetChildren())
		CalculatePose(dynamic_cast<Bone*>(child));

	transform = tmp;
}

bool Model::ProcessBone(aiNode* node, std::shared_ptr<Bone>& out)
{
	if(bonemap.find(node->mName.C_Str()) != bonemap.end())
	{
		auto it = std::find_if(bones.begin(), bones.end(), [&](auto& b) { return b->GetName() == std::string(node->mName.C_Str()); });
		if(it != bones.end())
			return false;
		out = std::make_shared<Bone>(bonemap[node->mName.C_Str()].first, node->mName.C_Str(), bonemap[node->mName.C_Str()].second);
		if(!anims.empty())
			if(anims[0]->GetKeyframes().find(out->GetName()) != anims[0]->GetKeyframes().end())
			{
				Keyframe kf = anims[0]->GetKeyframes()[out->GetName()];
				auto pos = kf.positions[0];
				auto rot = kf.rotations[0];
				rp3d::Transform tr;
				tr.setPosition({ pos.x, pos.y, pos.z });
				tr.setOrientation({ rot.x, rot.y, rot.z, rot.w });
				out->SetTransform(tr);
			}
			else out->SetTransform(ToRP3DTransform(toglm(node->mTransformation)).first);
		
		out->SavePoseAsIdle();

		for (int i = 0; i < node->mNumChildren; i++)
		{
			std::shared_ptr<Bone> child = nullptr;
			if(ProcessBone(node->mChildren[i], child))
			{
				bonesChildren.push_back(child);
				child->SetParent(out.get());
				out->AddChild(child.get());
			}
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
	data["immediateLoad"] = immLoad;

	data["rigidBody"]["active"] = body ? body->isActive() : false;
	data["rigidBody"]["collider"] = (int)cstype;
	data["rigidBody"]["type"] = (int)body->getType();

	return data;
}

void Model::Deserialize(Json::Value data, bool storeData)
{
	if(storeData)
	{
		this->data = data;
		return;
	}

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
	immLoad = data["immediateLoad"].asBool();

	if(body && !storeData)
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
