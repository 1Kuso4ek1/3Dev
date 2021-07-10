#include <3Dev.h>
#include <unordered_map>

class Material
{
public:
	void SetParameters(float parameters[], GLuint type);
	void SetMap(std::unordered_map<GLenum, float*> m);
	void operator=(Material& right);
	void Activate();
	
	std::unordered_map<GLenum, float*> GetMap();
private:
	std::unordered_map<GLenum, float*> mat;
};
