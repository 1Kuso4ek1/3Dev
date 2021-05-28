#include <3Dev.h>

std::pair<int, sf::Vector3f> collision(float x, float y, float z, Model& m, float p = 1)
{
	for (int i = 0; i < m.numVerts * 3; i += 3) {
		if(abs(x - ((m.GetPosition().x + m.vertexArray[i]))) <= p && abs(z - ((m.GetPosition().z + m.vertexArray[i + 2]))) <= p)
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
	for (int i = 0; i < m.numVerts * 3; i += 3 * mskip) {
		for (int j = 0; j < m1.numVerts * 3; j += 3 * m1skip) {
			if(fabsf(((m1.GetPosition().x + ((m1.vertexArray[j] * cos(m1.GetRotation().y * pi / 180)) * sin(m1.GetRotation().x * pi / 180)))) - ((m.GetPosition().x + m.vertexArray[i]))) <= p && fabsf(((m1.GetPosition().z + ((m1.vertexArray[j + 2] * cos(m1.GetRotation().y * pi / 180)) * cos(m1.GetRotation().z * pi / 180)))) - ((m.GetPosition().z + m.vertexArray[i + 2]))) <= p)
			{
				if ((m1.GetPosition().y + m1.vertexArray[j + 1]) <= (m.GetPosition().y + m.vertexArray[i + 1]) && (m1.GetPosition().y - m1.vertexArray[j + 1]) >= (m.GetPosition().y - m.vertexArray[i + 1])) {
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
