#include <Mesh.hpp>

Mesh::Mesh(std::vector<Vertex> data, std::vector<GLuint> indices, aiAABB aabb, /*std::vector<Bone> bones,*/ glm::mat4 transformation)
		   : data(data), indices(indices), aabb(aabb), /*bones(bones),*/ transformation(transformation)
{
	SetupBuffers();

	/*int nbones = 0;
	for(auto i : bones)
		nbones += BonesCount(i);
	pose.resize(nbones, glm::mat4(1.0));*/
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}

void Mesh::Draw()
{
    glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

std::vector<Vertex>& Mesh::GetData()
{
    return data;
}

std::vector<GLuint>& Mesh::GetIndices()
{
    return indices;
}

/*std::vector<Bone>& Mesh::GetBones()
{
	return bones;
}

std::vector<glm::mat4>& Mesh::GetPose()
{
	return pose;
}*/

glm::mat4 Mesh::GetTransformation()
{
	return transformation;
}

aiAABB Mesh::GetAABB()
{
	return aabb;
}

void Mesh::CreateCube()
{
	aabb = aiAABB({ -1, -1, -1 }, { 1, 1, 1 });
	data = 
	{
		{ glm::vec3(1, 1, -1), glm::vec3(0, 1, 0), glm::vec2(0, 0) },
		{ glm::vec3(-1, 1, -1), glm::vec3(0, 1, 0), glm::vec2(1, 0) },
		{ glm::vec3(-1, 1, 1), glm::vec3(0, 1, 0), glm::vec2(1, 1) },
		{ glm::vec3(1, 1, 1), glm::vec3(0, 1, 0), glm::vec2(0, 1) },
		{ glm::vec3(1, -1, 1), glm::vec3(0, 0, 1), glm::vec2(0, 0) },
		{ glm::vec3(1, 1, 1), glm::vec3(0, 0, 1), glm::vec2(1, 0) },
		{ glm::vec3(-1, 1, 1), glm::vec3(0, 0, 1), glm::vec2(1, 1) },
		{ glm::vec3(-1, -1, 1), glm::vec3(0, 0, 1), glm::vec2(0, 1) },
		{ glm::vec3(-1, -1, 1), glm::vec3(-1, 0, 0), glm::vec2(0, 0) },
		{ glm::vec3(-1, 1, 1), glm::vec3(-1, 0, 0), glm::vec2(1, 0) },
		{ glm::vec3(-1, 1, -1), glm::vec3(-1, 0, 0), glm::vec2(1, 1) },
		{ glm::vec3(-1, -1, -1), glm::vec3(-1, 0, 0), glm::vec2(0, 1) },
		{ glm::vec3(-1, -1, -1), glm::vec3(0, -1, 0), glm::vec2(0, 0) },
		{ glm::vec3(1, -1, -1), glm::vec3(0, -1, 0), glm::vec2(1, 0) },
		{ glm::vec3(1, -1, 1), glm::vec3(0, -1, 0), glm::vec2(1, 1) },
		{ glm::vec3(-1, -1, 1), glm::vec3(0, -1, 0), glm::vec2(0, 1) },
		{ glm::vec3(1, -1, -1), glm::vec3(1, 0, 0), glm::vec2(0, 0) },
		{ glm::vec3(1, 1, -1), glm::vec3(1, 0, 0), glm::vec2(1, 0) },
		{ glm::vec3(1, 1, 1), glm::vec3(1, 0, 0), glm::vec2(1, 1) },
		{ glm::vec3(1, -1, 1), glm::vec3(1, 0, 0), glm::vec2(0, 1) },
		{ glm::vec3(-1, -1, -1), glm::vec3(0, 0, -1), glm::vec2(0, 0) },
		{ glm::vec3(-1, 1, -1), glm::vec3(0, 0, -1), glm::vec2(1, 0) },
		{ glm::vec3(1, 1, -1), glm::vec3(0, 0, -1), glm::vec2(1, 1) },
		{ glm::vec3(1, -1, -1), glm::vec3(0, 0, -1), glm::vec2(0, 1) }
	};
	indices = 
	{
		0, 1, 2, 0, 2, 3, 4, 5, 6,
		4, 6, 7, 8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15, 16, 17, 18,
		16, 18, 19, 20, 21, 22, 20, 22, 23
	};
	SetupBuffers();
}

/*int Mesh::BonesCount(Bone& b)
{
	int n = 0; n++;
	for(auto i : b.children)
		n += BonesCount(i);
	return n;
}*/

void Mesh::SetupBuffers()
{
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);
	
	glBindVertexArray(vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(Vertex), &data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)12);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)24);
	glVertexAttribPointer(3, 4, GL_INT, GL_FALSE, sizeof(Vertex), (void*)32);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)48);
	
	glBindVertexArray(0);
}
