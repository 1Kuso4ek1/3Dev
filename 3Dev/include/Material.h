#include <3Dev.h>
#include <unordered_map>

class Material
{
public:
	Material();

	void SetParameters(std::vector<float> parameters, GLuint type);
	void SetMap(std::unordered_map<GLenum, std::vector<float>> m);
	void operator=(Material& right);
	void Activate();
	
	std::unordered_map<GLenum, std::vector<float>> GetMap();
private:
	std::unordered_map<GLenum, std::vector<float>> mat;
};
