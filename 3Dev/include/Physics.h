#include <3Dev.h>

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
			return sf::Vector3f((s.GetPosition().x + s.GetSize().x) - x, (s.GetPosition().y + s.GetSize().y) - y, (s.GetPosition().z + s.GetSize().z) - z);
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
