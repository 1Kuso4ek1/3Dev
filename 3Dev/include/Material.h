#include <3Dev.h>
#include <unordered_map>

class Material
{
public:
	void SetParameters(float parameters[], GLuint type);
	
	void Activate();
private:
	std::unordered_map<GLenum, float*> mat = 
	{
		{ GL_AMBIENT, (float[]){ 0.2, 0.2, 0.2, 1 } },
		{ GL_DIFFUSE, (float[]){ 0.8, 0.8, 0.8, 1 } },
		{ GL_SPECULAR, (float[]){ 0, 0, 0, 1 } },
		{ GL_EMISSION, (float[]){ 0, 0, 0, 1 } },
		{ GL_SHININESS, (float[]){ 0 } }
	};
};
