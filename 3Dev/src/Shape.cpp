#include <Shape.hpp>

Shape::Shape(const rp3d::Vector3& size, Material* mat, PhysicsManager* man, Shader* shader, Matrices* m)
			: size(size), tr({ 0, 0, 0 }, { 0, 0, 0, 1 }), mat(mat), man(man)
{
	if(shader) this->shader = shader;
	if(m) this->m = m;

	CheckOpacity();

	cube = std::make_shared<Mesh>();
	cube->CreateCube();

	if(man)
	{
		shape = man->CreateBoxShape(size);
		body = man->CreateRigidBody(tr);
		collider = body->addCollider(shape, rp3d::Transform::identity());
	}
}

void Shape::Draw(Camera* cam, std::vector<Light*> lights, bool transparencyPass)
{
	m->PushMatrix();

	if(body != nullptr) tr = body->getTransform();
	rp3d::Vector3 tmp; float a;
	tr.getOrientation().getRotationAngleAxis(a, tmp);

	m->Translate(toglm(tr.getPosition()));
	m->Rotate(a, glm::axis(toglm(tr.getOrientation()))); // Using toglm(tmp) as second argument breaks everything and gives the matrix of nan
	m->Scale(toglm(size));

	shader->Bind();
	mat->UpdateShader(shader);
	for(int i = 0; i < lights.size(); i++)
		lights[i]->Update(shader, i);
	if(cam)
		shader->SetUniform3f("campos", cam->GetPosition().x, cam->GetPosition().y, cam->GetPosition().z);
	shader->SetUniformMatrix4("transformation", glm::mat4(1.0));
	shader->SetUniform1i("drawTransparency", transparencyPass);
	shader->SetUniform1i("bones", 0);
	m->UpdateShader(shader);

	if(!transparent && transparencyPass)
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
    }

    cube->Draw();

    if(!transparent && transparencyPass)
    {
        glDisable(GL_CULL_FACE);
        glFrontFace(GL_CW);
    }

	mat->ResetShader(shader);

	m->PopMatrix();
}

void Shape::DrawSkybox()
{
	auto tex = mat->GetParameters();
	auto it = std::find_if(tex.begin(), tex.end(), [](auto& a) { return a.second == Material::Type::Cubemap; });
	auto shader = Renderer::GetInstance()->GetShader(Renderer::ShaderType::Skybox);

	if(it == tex.end()) return;

	m->PushMatrix();

	shader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, std::get<1>(it->first));

	shader->SetUniform1i("cubemap", 0);
	m->UpdateShader(shader);

	cube->Draw();

	m->PopMatrix();
}

void Shape::SetPosition(const rp3d::Vector3& position)
{
	tr.setPosition(position);
	if(body) body->setTransform(tr);
}

void Shape::SetOrientation(const rp3d::Quaternion& orientation)
{
	tr.setOrientation(orientation);
	if(body) body->setTransform(tr);
}

void Shape::SetSize(const rp3d::Vector3& size)
{
	this->size = size;
	if(body) shape->setHalfExtents(this->size);
}

void Shape::SetMaterial(Material* mat)
{
	this->mat = mat;
	CheckOpacity();
}

void Shape::SetShader(Shader* shader)
{
	this->shader = shader;
}

void Shape::SetPhysicsManager(PhysicsManager* man)
{
	this->man = man;
}

void Shape::CreateRigidBody()
{
	if(man) body = man->CreateRigidBody(tr);
}

void Shape::Move(const rp3d::Vector3& position)
{
	tr.setPosition(tr.getPosition() + position);
	if(body) body->setTransform(tr);
}

void Shape::Rotate(const rp3d::Quaternion& orientation)
{
	tr.setOrientation(orientation * tr.getOrientation());
	if(body) body->setTransform(tr);
}

void Shape::Expand(const rp3d::Vector3& size)
{
	this->size += size;
	if(body) shape->setHalfExtents(this->size);
}

void Shape::CheckOpacity()
{
    if(!mat->Contains(Material::Type::Opacity))
    {
        transparent = false;
        return;
    }

    auto p = mat->GetParameter(Material::Type::Opacity);
    if(std::holds_alternative<glm::vec3>(p))
        transparent = std::get<0>(p).x < 1.0;
    else if(std::holds_alternative<GLuint>(p))
        transparent = std::get<1>(p) > 0;
}

bool Shape::IsTransparent()
{
    return transparent;
}

Shader* Shape::GetShader()
{
	return shader;
}

rp3d::RigidBody* Shape::GetRigidBody()
{
	return body;
}

Material* Shape::GetMaterial()
{
	return mat;
}

rp3d::Vector3 Shape::GetPosition()
{
	return tr.getPosition();
}

rp3d::Quaternion Shape::GetOrientation()
{
	return tr.getOrientation();
}

rp3d::Vector3 Shape::GetSize()
{
	return size;
}

Json::Value Shape::Serialize()
{
	Json::Value data;

	auto pos = GetPosition();
	auto orient = GetOrientation();
	auto size = GetSize();

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

	data["rigidBody"]["active"] = body ? body->isActive() : false;

	return data;
}

void Shape::Deserialize(Json::Value data)
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

	if(body) body->setIsActive(data["rigidBody"]["active"].asBool());

	SetPosition(pos);
	SetOrientation(orient);
	SetSize(size);
}
