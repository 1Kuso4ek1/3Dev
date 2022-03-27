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
	
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	
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
