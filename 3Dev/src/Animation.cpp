#include <Animation.hpp>

Animation::Animation(const std::string& name) : name(name) {}

void Animation::SetName(const std::string& name)
{
    this->name = name;
}

void Animation::SetTPS(float tps)
{
    this->tps = tps;
}

void Animation::SetDuration(float duration)
{
    this->duration = duration;
}

void Animation::SetIsRepeated(bool repeat)
{
    this->repeat = repeat;
}

void Animation::AddKeyframe(std::string name, const Keyframe& keyframe)
{
    keyframes[name] = keyframe;
}

void Animation::Play()
{
    state = State::Playing;
    time.restart();
}

void Animation::Pause()
{
    state = State::Paused;
    lastTime = GetTime();
}

void Animation::Stop()
{
    state = State::Stopped;
}

std::unordered_map<std::string, std::pair<rp3d::Transform, rp3d::Vector3>> Animation::Update()
{
    std::unordered_map<std::string, std::pair<rp3d::Transform, rp3d::Vector3>> actions;

    if(state != State::Stopped)
        for(auto& [name, keyframe] : keyframes)
        {
            float time = 0;
            if(state == State::Playing)
            {
                time = GetTime();
                if(lastTime != 0)
                    time += lastTime;
            }
            else time = lastTime;

            if(state == State::Playing && time >= duration)
            {
                if(repeat)
                {
                    time = this->time.restart().asSeconds() * tps;
                    lastTime = 0;
                }
                else
                {
                    time = duration - 0.01;
                    state = State::Paused;
                    lastTime = duration - 0.01;
                }
            }

            float dt = fmod(time, duration);
            auto fraction = TimeFraction(keyframe.rotStamps, dt);

            glm::vec3 pos = glm::mix(keyframe.positions[fraction.first - 1], keyframe.positions[fraction.first], fraction.second);
            glm::quat rot = glm::slerp(keyframe.rotations[fraction.first - 1], keyframe.rotations[fraction.first], fraction.second);
            glm::vec3 scale = glm::mix(keyframe.scales[fraction.first - 1], keyframe.scales[fraction.first], fraction.second);

            actions[name] = { { { pos.x, pos.y, pos.z }, { rot.x, rot.y, rot.z, rot.w } }, { scale.x, scale.y, scale.z } };
        }

    return actions;
}

std::unordered_map<std::string, Keyframe>& Animation::GetKeyframes()
{
    return keyframes;
}

std::string Animation::GetName()
{
    return name;
}

float Animation::GetTime()
{
    return time.getElapsedTime().asSeconds() * tps;
}

Animation::State Animation::GetState()
{
    return state;
}
