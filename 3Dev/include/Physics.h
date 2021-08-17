#include <3Dev.h>

sf::Vector3f rotate(sf::Vector3f pos, sf::Vector3f rot)
{
	float x = pos.x * (cos(rot.x * pi / 180) * cos(rot.y * pi / 180)) + pos.y * (cos(rot.x * pi / 180) * sin(rot.y * pi / 180) * sin(rot.z * pi / 180) - sin(rot.x * pi / 180) * cos(rot.z * pi / 180)) + pos.z * (cos(rot.x * pi / 180) * sin(rot.y * pi / 180) * cos(rot.z * pi / 180) + sin(rot.y * pi / 180) * sin(rot.z * pi / 180));
	float y = pos.x * (sin(rot.x * pi / 180) * cos(rot.y * pi / 180)) + pos.y * (sin(rot.x * pi / 180) * sin(rot.y * pi / 180) * sin(rot.z * pi / 180) + cos(rot.x * pi / 180) * cos(rot.z * pi / 180)) + pos.z * (sin(rot.x * pi / 180) * sin(rot.y * pi / 180) * cos(rot.z * pi / 180) - cos(rot.y * pi / 180) * sin(rot.z * pi / 180));
	float z = pos.x * (-sin(rot.y * pi / 180)) + pos.y * (cos(rot.y * pi / 180) * sin(rot.z * pi / 180)) + pos.z * (cos(rot.y * pi / 180) * cos(rot.z * pi / 180));
	return sf::Vector3f(x, y, z);
}

std::vector<float> rotatemodel(Model& m)
{
	std::vector<float> ret;
	for (int i = 0; i < m.numVerts * 3; i += 3) {
		sf::Vector3f res(rotate(sf::Vector3f(m.vertexArray[i], m.vertexArray[i + 1], m.vertexArray[i + 2]), m.GetRotation()));
		ret.push_back(res.x); ret.push_back(res.y); ret.push_back(res.z);
	}
	return ret;
}

std::pair<int, sf::Vector3f> collision(float x, float y, float z, Model& m, float p = 1)
{
	std::vector<float> rotvert = rotatemodel(m);
	for (int i = 0; i < m.numVerts * 3; i += 3) {
		if(std::abs(x - ((m.GetPosition().x + rotvert[i]))) <= p && std::abs(z - ((m.GetPosition().z + rotvert[i + 2]))) <= p)
		{
			if (y <= (m.GetPosition().y + rotvert[i + 1])) {
				return std::make_pair(i, sf::Vector3f(((m.GetPosition().x + rotvert[i])) - x, ((m.GetPosition().y + rotvert[i + 1])) - y, ((m.GetPosition().z + rotvert[i + 2])) - z));
			}
		}
	}
	return std::make_pair(-1, sf::Vector3f(0, 0, 0));
}

std::pair<std::vector<std::pair<int, sf::Vector3f>>, std::vector<std::pair<int, sf::Vector3f>>> collision(Model& m1, Model& m, float p = 1, int m1skip = 1, int mskip = 1)
{
	std::vector<std::pair<int, sf::Vector3f>> v, v1;
	std::vector<float> rotvert = rotatemodel(m1), rotvert1 = rotatemodel(m);
	
	for (int i = 0; i < m.numVerts * 3; i += 3 * mskip) {
		for (int j = 0; j < m1.numVerts * 3; j += 3 * m1skip) {
			if(std::abs(m1.GetPosition().x + rotvert[j] - (m.GetPosition().x + rotvert1[i])) <= p && std::abs(m1.GetPosition().z + rotvert[j + 2] - (m.GetPosition().z + rotvert1[i + 2])) <= p)
			{
				if ((m1.GetPosition().y + rotvert[j + 1]) <= (m.GetPosition().y + rotvert1[i + 1]) && (m1.GetPosition().y - rotvert[j + 1]) >= (m.GetPosition().y - rotvert1[i + 1])) {
					v.push_back(std::make_pair(j, sf::Vector3f(0, (m.GetPosition().y + rotvert1[i + 1]) - (m1.GetPosition().y + rotvert[j + 1]), 0)));
					v1.push_back(std::make_pair(i, sf::Vector3f(0, (m.GetPosition().y - rotvert1[i + 1]) - (m1.GetPosition().y - rotvert[j + 1]), 0)));
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
