#include <3Dev.h>

sf::Vector3f rotate(sf::Vector3f pos, sf::Vector3f rot)
{
	float x = pos.x * (cos(rot.x * pi / 180) * cos(rot.y * pi / 180)) + pos.y * (cos(rot.x * pi / 180) * sin(rot.y * pi / 180) * sin(rot.z * pi / 180) - sin(rot.x * pi / 180) * cos(rot.z * pi / 180)) + pos.z * (cos(rot.x * pi / 180) * sin(rot.y * pi / 180) * cos(rot.z * pi / 180) + sin(rot.y * pi / 180) * sin(rot.z * pi / 180));
	float y = pos.x * (sin(rot.x * pi / 180) * cos(rot.y * pi / 180)) + pos.y * (sin(rot.x * pi / 180) * sin(rot.y * pi / 180) * sin(rot.z * pi / 180) + cos(rot.x * pi / 180) * cos(rot.z * pi / 180)) + pos.z * (sin(rot.x * pi / 180) * sin(rot.y * pi / 180) * cos(rot.z * pi / 180) - cos(rot.y * pi / 180) * sin(rot.z * pi / 180));
	float z = pos.x * (-sin(rot.y * pi / 180)) + pos.y * (cos(rot.y * pi / 180) * sin(rot.z * pi / 180)) + pos.z * (cos(rot.y * pi / 180) * cos(rot.z * pi / 180));
	return sf::Vector3f(x, y, z);
}

std::pair<int, sf::Vector3f> collision(float x, float y, float z, Model& m, float p = 1)
{
	for (int i = 0; i < m.numVerts * 3; i += 3) {
		if(std::abs(x - ((m.GetPosition().x + m.vertexArray[i]))) <= p && std::abs(z - ((m.GetPosition().z + m.vertexArray[i + 2]))) <= p)
		{
			if (y <= (m.GetPosition().y + m.vertexArray[i + 1]) && y >= (m.GetPosition().y - m.vertexArray[i + 1])) {
				return std::make_pair(i, sf::Vector3f(((m.GetPosition().x + m.vertexArray[i])) - x, ((m.GetPosition().y + m.vertexArray[i + 1])) - y, ((m.GetPosition().z + m.vertexArray[i + 2])) - z));
			}
		}
	}
	return std::make_pair(-1, sf::Vector3f(0, 0, 0));
}

std::pair<std::vector<int>, std::vector<int>> collision(Model& m1, Model& m, float p = 1, int m1skip = 1, int mskip = 1)
{
	std::vector<int> v;
	std::vector<int> v1;
	std::vector<float> rotvert;
	for (int j = 0; j < m1.numVerts * 3; j += 3) {
		sf::Vector3f res(rotate(sf::Vector3f(m1.vertexArray[j], m1.vertexArray[j + 1], m1.vertexArray[j + 2]), m1.GetRotation()));
		rotvert.push_back(res.x);
		rotvert.push_back(res.y);
		rotvert.push_back(res.z);
	}
	for (int i = 0; i < m.numVerts * 3; i += 3 * mskip) {
		for (int j = 0; j < m1.numVerts * 3; j += 3 * m1skip) {
			if(std::abs(m1.GetPosition().x + rotvert[j] - (m.GetPosition().x + m.vertexArray[i])) <= p && std::abs(m1.GetPosition().z + rotvert[j + 2] - (m.GetPosition().z + m.vertexArray[i + 2])) <= p)
			{
				if ((m1.GetPosition().y + rotvert[j + 1]) <= (m.GetPosition().y + m.vertexArray[i + 1]) && (m1.GetPosition().y - rotvert[j + 1]) >= (m.GetPosition().y - m.vertexArray[i + 1])) {
					v.push_back(j);
					v1.push_back(i);
				}
			}
		}
	}
	return std::make_pair(v, v1);
}

bool collision(Model& m, float x, float y, float z, float w = 0, float h = 0, float d = 0)
{
	for (int i = 0; i < m.numVerts * 3; i += 3) {
		if((m.GetPosition().x + m.vertexArray[i] <= x + w && m.GetPosition().x + m.vertexArray[i] >= x - w) && (m.GetPosition().z + m.vertexArray[i + 2] <= z + d && m.GetPosition().z + m.vertexArray[i + 2] >= z - d))
		{
			if (m.GetPosition().y + m.vertexArray[i + 1] <= y + h && m.GetPosition().y + m.vertexArray[i + 1] >= y - h) {
				return true;
			}
		}
	}
	return false;
}

bool collision(float x, float y, float z, float xx, float yy, float zz, float w = 0, float h = 0, float d = 0)
{
	if((x <= xx + w && x >= xx - w) && (z <= zz + d && z >= zz - d))
	{
		if (y <= yy + h && y >= yy - h) {
			return true;
		}
	}
	return false;
}

sf::Vector3f collision(float x, float y, float z, Shape s)
{
	if((x <= s.GetPosition().x + s.GetSize().x && x >= s.GetPosition().x - s.GetSize().x) && (z <= s.GetPosition().z + s.GetSize().z && z >= s.GetPosition().z - s.GetSize().z))
	{
		if (y <= s.GetPosition().y + s.GetSize().y && y >= s.GetPosition().y - s.GetSize().y)
		{
			return sf::Vector3f((s.GetPosition().z + s.GetSize().z) - z, (s.GetPosition().y + s.GetSize().y) - y, (s.GetPosition().z + s.GetSize().z) - z);
		}
	}
	return sf::Vector3f(0, 0, 0);
}

bool collision(Model& m, Shape s)
{
	for (int i = 0; i < m.numVerts * 3; i += 3) {
		if((m.GetPosition().x + m.vertexArray[i] <= s.GetPosition().x + s.GetSize().x && m.GetPosition().x + m.vertexArray[i] >= s.GetPosition().x - s.GetSize().x) && (m.GetPosition().z + m.vertexArray[i + 2] <= s.GetPosition().z + s.GetSize().z && m.GetPosition().z + m.vertexArray[i + 2] >= s.GetPosition().z - s.GetSize().z))
		{
			if (m.GetPosition().y + m.vertexArray[i + 1] <= s.GetPosition().y + s.GetSize().y && m.GetPosition().y + m.vertexArray[i + 1] >= s.GetPosition().y - s.GetSize().y) {
				return true;
			}
		}
	}
	return false;
}
