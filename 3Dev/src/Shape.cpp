#include "Shape.h"

Shape::Shape(const rp3d::Vector3& position, const rp3d::Vector3& size, const rp3d::Quaternion& orient, PhysicsManager* man, Shader* shader, Material* mat, Matrices* m, GLuint cubemap) : size(size), tr(position, orient), shader(shader), mat(mat), cubemap(cubemap), m(m)
{
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);
	
	glBindVertexArray(vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)12);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)24);
	
	glBindVertexArray(0);

	if(man != nullptr)
	{
		shape = man->CreateBoxShape(size);
		body = man->CreateRigidBody(tr);
		collider = body->addCollider(shape, rp3d::Transform::identity());
	}
}

Shape::~Shape()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}

void Shape::Draw(Camera& cam, std::vector<Light> lights) 
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
		lights[i].Update(shader, i);
	shader->SetUniform1f("shininess", mat->GetShininess());
	shader->SetUniform3f("campos", cam.GetPosition().x, cam.GetPosition().y, cam.GetPosition().z);
	shader->SetUniform1i("bones", 0);
	m->UpdateShader(shader);
	
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	mat->ResetShader(shader);
	
	m->PopMatrix();
}

void Shape::DrawSkybox()
{
	if(cubemap == 0) return; // There's no cubemap!

	m->PushMatrix();
	
	m->Translate(toglm(tr.getPosition()));
	m->Scale(toglm(size));

	shader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	
	shader->SetUniform1i("cubemap", 0);
	//std::cout << "skybox" << std::endl;
	m->UpdateShader(shader);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

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

rp3d::RigidBody* Shape::GetRigidBody()
{
	return body;
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
