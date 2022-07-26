#include "Shape.hpp"

Shape::Shape(const rp3d::Vector3& size, Material* mat, PhysicsManager* man, Shader* shader, Matrices* m)
			: size(size), tr({ 0, 0, 0 }, { 0, 0, 0, 1 }), mat(mat)
{
	if(shader) this->shader = shader;
	if(m) this->m = m;

	cube = std::make_shared<Mesh>();
	cube->CreateCube();

	if(man)
	{
		shape = man->CreateBoxShape(size);
		body = man->CreateRigidBody(tr);
		collider = body->addCollider(shape, rp3d::Transform::identity());
	}
}

void Shape::Draw(Camera* cam, std::vector<Light*> lights)
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
	shader->SetUniform1i("bones", 0);
	m->UpdateShader(shader);
	
	cube->Draw();

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
	if(body != nullptr) body->setTransform(tr);
}

void Shape::SetOrientation(const rp3d::Quaternion& orientation)
{
	tr.setOrientation(orientation);
	if(body != nullptr) body->setTransform(tr);
}

void Shape::SetSize(const rp3d::Vector3& size)
{
	this->size = size;
}

void Shape::SetMaterial(Material* mat)
{
	this->mat = mat;
}

void Shape::SetShader(Shader* shader)
{
	this->shader = shader;
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
