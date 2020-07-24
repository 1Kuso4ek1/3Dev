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

void Animation::DrawAnimation(float speed, GLuint texture)
{
	if(current_frame >= frames - 1) {
		current_frame = 1;
	}
	m[(int)current_frame].Draw(texture);
	current_frame += speed;
}

void Animation::DrawAnimation(float speed)
{
	if (current_frame >= frames - 1) {
		current_frame = 1;
	}
	m[(int)current_frame].Draw(false);
	current_frame += speed;
}
