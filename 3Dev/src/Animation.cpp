#include "Animation.h"

Animation::Animation(std::string filename, std::string texture, int frames, float speed, std::string ID, float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz) : frames(frames), position(x, y, z), speed(speed), ID(ID), filename(filename), texture(texture), rotation(rx, ry, rz), size(sx, sy, sz)
{
	Load(filename, texture, frames);
}

Animation::Animation(std::string filename, GLuint texture, int frames, float speed, std::string ID, float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz) : frames(frames), position(x, y, z), speed(speed), ID(ID), filename(filename), animationTexture(texture), rotation(rx, ry, rz), size(sx, sy, sz)
{
	Load(filename, frames);
}

Animation::Animation(std::string filename, std::string texture, int frames, float speed) : frames(frames), speed(speed), position(0, 0, 0), rotation(0, 0, 0), size(1, 1, 1), ID("")
{
	Load(filename, texture, frames);
}

Animation::Animation(std::string filename, GLuint texture, int frames, float speed) : animationTexture(texture), frames(frames), speed(speed), position(0, 0, 0), rotation(0, 0, 0), size(1, 1, 1), ID("")
{
	Load(filename, frames);
}

Animation::Animation() : speed(1), position(0, 0, 0), rotation(0, 0, 0), size(1, 1, 1), ID("") {}

Animation::~Animation()
{
	delete[] m;
}

void Animation::Load(std::string filename, std::string texture, int frames)
{
	this->frames = frames;
	animationTexture = LoadTexture(texture);
	m = new Model[frames - 1];
	for (int i = 0; i < frames - 1; i++) 
	{
		if (i <= 9)
		{
			if (!m[i].Load(filename + "_00000" + std::to_string(i) + ".obj")) 
			{
				delete[] m;
			}
		}
		else if (i <= 99 && i > 9) 
		{
			if (!m[i].Load(filename + "_0000" + std::to_string(i) + ".obj")) 
			{
				delete[] m;
			}
		}
		else if (i <= 999 && i > 99 && i > 9) 
		{
			if (!m[i].Load(filename + "_000" + std::to_string(i) + ".obj")) 
			{
				delete[] m;
			}
		}
		m[i].SetPosition(position.x, position.y, position.z);
		m[i].SetRotation(rotation.x, rotation.y, rotation.z);
		m[i].SetSize(size.x, size.y, size.z);
	}
}

void Animation::Load(std::string filename, int frames)
{
	this->frames = frames;
	m = new Model[frames - 1];
	for (int i = 0; i < frames - 1; i++) 
	{
		if (i <= 9)
		{
			if (!m[i].Load(filename + "_00000" + std::to_string(i) + ".obj")) 
			{
				delete[] m;
			}
		}
		else if (i <= 99 && i > 9) 
		{
			if (!m[i].Load(filename + "_0000" + std::to_string(i) + ".obj")) 
			{
				delete[] m;
			}
		}
		else if (i <= 999 && i > 99 && i > 9) 
		{
			if (!m[i].Load(filename + "_000" + std::to_string(i) + ".obj")) 
			{
				delete[] m;
			}
		}
		m[i].SetPosition(position.x, position.y, position.z);
		m[i].SetRotation(rotation.x, rotation.y, rotation.z);
		m[i].SetSize(size.x, size.y, size.z);
	}
}

void Animation::DrawAnimation(float time)
{
	if ((int)current_frame >= frames - 1 && loop && state == Playing) 
	{
		current_frame = 1;
	}
	else if((int)current_frame >= frames - 1 && !loop && state == Playing) { m[(int)current_frame - 1].Draw(animationTexture); return; }
	m[(int)current_frame].Draw(animationTexture);
	if(state == Playing) current_frame += speed * time;
}

void Animation::DrawFrame(int frame)
{
	m[frame].Draw(animationTexture);
}

void Animation::Restart()
{
	current_frame = 1;
}

void Animation::Stop()
{
	state = Stopped;
	Restart();
}

void Animation::Pause()
{
	state = Paused;
}

void Animation::Play()
{
	state = Playing;
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

void Animation::SetID(std::string ID)
{
	this->ID = ID;
}

void Animation::SetLoop(bool loop)
{
	this->loop = loop;
}

void Animation::SetSpeed(float speed)
{
	this->speed = speed;
}

void Animation::SetTexture(GLuint texture)
{
	animationTexture = texture;
}

void Animation::SetMaterial(Material mat)
{
	this->mat = mat;
	for (int i = 0; i < frames - 1; i++) {
		m[i].SetMaterial(this->mat);
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

int Animation::GetCurrentFrame()
{
	return (int)current_frame;
}

float Animation::GetSpeed()
{
	return speed;
}

Animation::State Animation::GetState()
{
	return state;
}

Material Animation::GetMaterial()
{
	return mat;
}
