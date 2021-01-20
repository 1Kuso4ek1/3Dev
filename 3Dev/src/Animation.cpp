#include "Animation.h"

Animation::Animation(int frames, std::string filename, std::string texture, float x, float y, float z, float speed, std::string ID, float rx, float ry, float rz, float sx, float sy, float sz) : frames(frames), position(x, y, z), speed(speed), ID(ID), filename(filename), texture(texture), rotation(rx, ry, rz), size(sx, sy, sz) {
	animationTexture = LoadTexture(texture);
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
		m[i].SetRotation(rx, ry, rz);
		m[i].SetSize(sx, sy, sz);
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
	m[(int)current_frame].Draw(animationTexture);
	current_frame += speed * time;
}

void Animation::DrawFrame(int frame)
{
	m[frame].Draw(animationTexture);
}

void Animation::SetPosition(float x, float y, float z)
{
	position = sf::Vector3f(x, y, z);

	for (int i = 0; i < frames - 1; i++) {
		m[i].SetPosition(x, y, z);
	}
}

void Animation::SetSize(float sizeX, float sizeY, float sizeZ)
{
	size = sf::Vector3f(sizeX, sizeY, sizeZ);

	for (int i = 0; i < frames - 1; i++) {
		m[i].SetSize(sizeX, sizeY, sizeZ);
	}
}

void Animation::SetRotation(float rotationX, float rotationY, float rotationZ)
{
	rotation = sf::Vector3f(rotationX, rotationY, rotationZ);

	for (int i = 0; i < frames - 1; i++) {
		m[i].SetRotation(rotationX, rotationY, rotationZ);
	}
}

void Animation::AddPosition(float x, float y, float z)
{
	SetPosition(position.x + x, position.y + y, position.z + z);
}

void Animation::AddRotation(float rotationX, float rotationY, float rotationZ)
{
	SetRotation(rotation.x + rotationX, rotation.y + rotationY, rotation.z + rotationZ);
}

void Animation::AddSize(float sizeX, float sizeY, float sizeZ)
{
	SetSize(size.x + sizeX, size.y + sizeY, size.z + sizeZ);
}

sf::Vector3f Animation::GetPosition() {
	return position;
}

sf::Vector3f Animation::GetRotation() {
	return rotation;
}

sf::Vector3f Animation::GetSize() {
	return size;
}

std::string Animation::GetID() {
	return ID;
}

std::string Animation::GetFilename() {
	return filename;
}

std::string Animation::GetTextureFilename() {
	return texture;
}

int Animation::GetFrames() {
	return frames;
}
