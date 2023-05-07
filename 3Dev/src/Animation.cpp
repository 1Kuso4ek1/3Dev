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

void Animation::SetLastTime(float lastTime)
{
    this->lastTime = lastTime;
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
    lastTime = 0.01;
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
                    time = duration;
                    state = State::Paused;
                    lastTime = duration;
                }
            }

            float dt = fmod(time == 0 ? time + 0.01 : (time == duration ? time - 0.01 : time), duration);

            auto posFraction = TimeFraction(keyframe.posStamps, dt);
            auto rotFraction = TimeFraction(keyframe.rotStamps, dt);
            auto scaleFraction = TimeFraction(keyframe.scaleStamps, dt);

            glm::vec3 pos = glm::mix(keyframe.positions[posFraction.first - 1], keyframe.positions[posFraction.first], posFraction.second);
            glm::quat rot = glm::slerp(keyframe.rotations[rotFraction.first - 1], keyframe.rotations[rotFraction.first], rotFraction.second);
            glm::vec3 scale = glm::mix(keyframe.scales[scaleFraction.first - 1], keyframe.scales[scaleFraction.first], scaleFraction.second);

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

bool Animation::IsRepeated()
{
    return repeat;
}

float Animation::GetTime()
{
    return time.getElapsedTime().asSeconds() * tps;
}

float Animation::GetLastTime()
{
    return lastTime;
}

float Animation::GetDuration()
{
    return duration;
}

float Animation::GetTPS()
{
    return tps;
}

Animation::State Animation::GetState()
{
    return state;
}

Json::Value Animation::Serialize()
{
    Json::Value data;

    data["duration"] = duration;
    data["tps"] = tps;
    data["repeat"] = repeat;

    int counter = 0;
    for(auto& [name, kf] : keyframes)
    {
        data["keyframes"][counter]["name"] = name;
        for(int i = 0; i < kf.posStamps.size(); i++)
            data["keyframes"][counter]["posStamps"][i] = kf.posStamps[i];
        for(int i = 0; i < kf.rotStamps.size(); i++)
            data["keyframes"][counter]["rotStamps"][i] = kf.rotStamps[i];
        for(int i = 0; i < kf.scaleStamps.size(); i++)
            data["keyframes"][counter]["scaleStamps"][i] = kf.scaleStamps[i];
        for(int i = 0; i < kf.positions.size(); i++)
        {
            data["keyframes"][counter]["positions"][i]["x"] = kf.positions[i].x;
            data["keyframes"][counter]["positions"][i]["y"] = kf.positions[i].y;
            data["keyframes"][counter]["positions"][i]["z"] = kf.positions[i].z;
        }
        for(int i = 0; i < kf.rotations.size(); i++)
        {
            data["keyframes"][counter]["rotations"][i]["x"] = kf.rotations[i].x;
            data["keyframes"][counter]["rotations"][i]["y"] = kf.rotations[i].y;
            data["keyframes"][counter]["rotations"][i]["z"] = kf.rotations[i].z;
            data["keyframes"][counter]["rotations"][i]["w"] = kf.rotations[i].w;
        }
        for(int i = 0; i < kf.scales.size(); i++)
        {
            data["keyframes"][counter]["scales"][i]["x"] = kf.scales[i].x;
            data["keyframes"][counter]["scales"][i]["y"] = kf.scales[i].y;
            data["keyframes"][counter]["scales"][i]["z"] = kf.scales[i].z;
        }

        counter++;
    }

    return data;
}

void Animation::Deserialize(Json::Value data)
{
    duration = data["duration"].asFloat();
    tps = data["tps"].asFloat();
    repeat = data["repeat"].asBool();

    for(auto& i : data["keyframes"])
    {
        Keyframe kf;
        
        for(auto& j : i["posStamps"])
            kf.posStamps.push_back(j.asFloat());
        for(auto& j : i["rotStamps"])
            kf.rotStamps.push_back(j.asFloat());
        for(auto& j : i["scaleStamps"])
            kf.scaleStamps.push_back(j.asFloat());
        for(auto& j : i["positions"])
        {
            glm::vec3 v(j["x"].asFloat(), j["y"].asFloat(), j["z"].asFloat());
            kf.positions.push_back(v);
        }
        for(auto& j : i["rotations"])
        {
            glm::quat q(j["w"].asFloat(), j["x"].asFloat(), j["y"].asFloat(), j["z"].asFloat());
            kf.rotations.push_back(q);
        }
        for(auto& j : i["scales"])
        {
            glm::vec3 v(j["x"].asFloat(), j["y"].asFloat(), j["z"].asFloat());
            kf.scales.push_back(v);
        }

        keyframes[i["name"].asString()] = kf;
    }
}
