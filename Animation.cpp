#include "Animation.h"

Animation::Animation(int frames) : frames(frames) {
	m = new Model[frames];
}

Animation::~Animation()
{
	delete[] m;
}

void Animation::LoadAnimation(std::string filename)
{
	for (int i = 0; i < frames; i++) {
		if (i <= 9) {
			if (!m[i].Load(filename + "_00000" + std::to_string(i) + ".obj")) {
				Animation::~Animation();
			}
		}
		else if (i <= 99 && i > 9) {
			if (!m[i].Load(filename + "_0000" + std::to_string(i) + ".obj")) {
				Animation::~Animation();
			}
		}
		else if (i <= 999 && i > 99 && i > 9) {
			if (!m[i].Load(filename + "_000" + std::to_string(i) + ".obj")) {
				Animation::~Animation();
			}
		}
	}
}

void Animation::DrawAnimation(int frame, GLuint texture)
{
	m[frame].Draw(texture);
}

void Animation::DrawAnimation(int frame)
{
	m[frame].Draw(false);
}
