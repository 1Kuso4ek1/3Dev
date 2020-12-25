#include "Animation.h"

Animation::Animation(int frames, std::string filename, std::string stexture, float x, float y, float z, float speed, std::string ID, float rx, float ry, float rz, float sx, float sy, float sz) : frames(frames), x(x), y(y), z(z), speed(speed), ID(ID), filename(filename), stexture(stexture), rotationX(rx), rotationY(ry), rotationZ(rz), sizeX(sx), sizeY(sy), sizeZ(sz) {
	this->texture = LoadTexture(stexture);
	m = new Model[frames - 1];
	for (int i = 0; i < frames - 1; i++) {
		if (i <= 9) {
			if (!m[i].Load(filename + "_00000" + std::to_string(i) + ".obj")) {
				delete[] m;
			}
		}
		else if (i <= 99 && i > 9) {
			if (!m[i].Load(filename + "_0000" + std::to_string(i) + ".obj")) {
				delete[] m;
			}
		}
		else if (i <= 999 && i > 99 && i > 9) {
			if (!m[i].Load(filename + "_000" + std::to_string(i) + ".obj")) {
				delete[] m;
			}
		}
		m[i].SetPosition(x, y, z);
	}
}

Animation::~Animation()
{
	delete[] m;
}

void Animation::DrawAnimation(float time)
{
	if ((int)current_frame >= frames - 1) {
		current_frame = 1;
	}
	m[(int)current_frame].Draw(texture);
	current_frame += speed * time;
}

void Animation::SetPosition(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;

	for (int i = 0; i < frames - 1; i++) {
		m[i].SetPosition(x, y, z);
	}
}

void Animation::SetSize(float sizeX, float sizeY, float sizeZ)
{
	this->sizeX = sizeX;
	this->sizeY = sizeY;
	this->sizeZ = sizeZ;

	for (int i = 0; i < frames - 1; i++) {
		m[i].SetSize(sizeX, sizeY, sizeZ);
	}
}

void Animation::SetRotation(float rotationX, float rotationY, float rotationZ)
{
	this->rotationX = rotationX;
	this->rotationY = rotationY;
	this->rotationZ = rotationZ;

	for (int i = 0; i < frames - 1; i++) {
		m[i].SetRotation(rotationX, rotationY, rotationZ);
	}
}

void Animation::AddPosition(float x, float y, float z)
{
	SetPosition(this->x + x, this->y + y, this->z + z);
}

void Animation::AddRotation(float rotationX, float rotationY, float rotationZ)
{
	SetRotation(this->rotationX + rotationX, this->rotationY + rotationY, this->rotationZ + rotationZ);
}

void Animation::AddSize(float sizeX, float sizeY, float sizeZ)
{
	SetSize(this->sizeX + sizeX, this->sizeY + sizeY, this->sizeZ + sizeZ);
}
