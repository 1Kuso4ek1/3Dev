#pragma once
#include "Utils.hpp"

struct Keyframe
{
	std::vector<float> posStamps;
	std::vector<float> rotStamps;
	std::vector<float> scaleStamps;

	std::vector<glm::vec3> positions;
	std::vector<glm::quat> rotations;
	std::vector<glm::vec3> scales;
};

class Animation
{
public:
    enum class State
	{
		Stopped,
		Playing,
		Paused
	};

    Animation(const std::string& name);

    void SetName(const std::string& name);
    void SetTPS(float tps);
    void SetDuration(float duration);
    void SetIsRepeated(bool repeat);

    void AddKeyframe(std::string name, const Keyframe& keyframe);

    void Play();
    void Pause();
    void Stop();

    std::unordered_map<std::string, std::pair<rp3d::Transform, rp3d::Vector3>> Update();

    std::unordered_map<std::string, Keyframe>& GetKeyframes();
    std::string GetName();
	float GetTime();
	float GetDuration();
	float GetTPS();
    State GetState();

	Json::Value Serialize();
	void Deserialize(Json::Value data);

private:
    std::string name;

    State state = State::Stopped;
	bool repeat = true;

	float duration = 0.0;
	float tps = 30.0;
	float lastTime = 0.0;

	std::unordered_map<std::string, Keyframe> keyframes;

	sf::Clock time;
};
