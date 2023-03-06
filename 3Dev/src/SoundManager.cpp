#include <SoundManager.hpp>

void ListenerWrapper::SetPosition(rp3d::Vector3 pos)
{
    sf::Listener::setPosition(pos.x, pos.y, pos.z);
}

void ListenerWrapper::SetUpVector(rp3d::Vector3 vec)
{
    sf::Listener::setUpVector(vec.x, vec.y, vec.z);
}

void ListenerWrapper::SetOrientation(rp3d::Quaternion orient)
{
    auto vec = orient * rp3d::Vector3(0, 0, -1);
    sf::Listener::setDirection(vec.x, vec.y, vec.z);
}

void ListenerWrapper::SetGlobalVolume(float volume)
{
    sf::Listener::setGlobalVolume(volume);
}

void SoundManager::LoadSound(std::string filename, std::string name)
{
    int nameCount = std::count_if(buffers.begin(), buffers.end(), [&](auto& p)
                    { return p.name.find(name) != std::string::npos; });

    name += (nameCount ? std::to_string(nameCount) : "");
    buffers.emplace_back(filename, name);
}

void SoundManager::LoadSound(sf::SoundBuffer& buffer, std::string name)
{
    int nameCount = std::count_if(buffers.begin(), buffers.end(), [&](auto& p)
                    { return p.name.find(name) != std::string::npos; });

    name += (nameCount ? std::to_string(nameCount) : "");
    buffers.emplace_back(buffer, name);
}

void SoundManager::Play(std::string name, int id)
{
    auto s = sounds.find(name + std::to_string(id));
    if(s != sounds.end())
        if(s->second->getStatus() == sf::Sound::Status::Paused)
        {
            s->second->play();
            return;
        }
    if(sounds.size() > 254) sounds.erase(sounds.begin());
    auto it = std::find(buffers.begin(), buffers.end(), name);
    sounds[name + std::to_string(id)] = std::make_shared<sf::Sound>(it->buffer);
    sounds[name + std::to_string(id)]->play();
    it->UpdateActiveSound(sounds, id);
}

void SoundManager::PlayAt(std::string name, int id, rp3d::Vector3 pos)
{
    if(sounds.size() > 254) sounds.erase(sounds.begin());
    auto it = std::find(buffers.begin(), buffers.end(), name);
    sounds[name + std::to_string(id)] = std::make_shared<sf::Sound>(it->buffer);
    sounds[name + std::to_string(id)]->play();
    it->UpdateActiveSound(sounds, id);
    sounds[name + std::to_string(id)]->setRelativeToListener(false);
    sounds[name + std::to_string(id)]->setPosition(pos.x, pos.y, pos.z);
}

void SoundManager::PlayMono(std::string name, int id)
{
    if(sounds.size() > 254) sounds.erase(sounds.begin());
    auto it = std::find(buffers.begin(), buffers.end(), name);
    sounds[name + std::to_string(id)] = std::make_shared<sf::Sound>(it->buffer);
    sounds[name + std::to_string(id)]->play();
    it->UpdateActiveSound(sounds, id);
    sounds[name + std::to_string(id)]->setRelativeToListener(true);
    sounds[name + std::to_string(id)]->setPosition(0, 0, 0);
}

void SoundManager::Stop(std::string name, int id)
{
    auto it = sounds.find(name + std::to_string(id));
    if(it != sounds.end())
    {
        it->second->stop();
        it->second->resetBuffer();
        sounds.erase(it);
    }
}

void SoundManager::Pause(std::string name, int id)
{
    auto it = sounds.find(name + std::to_string(id));
    if(it != sounds.end())
        it->second->pause();
}

void SoundManager::SetName(std::string name, std::string newName)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
	if(it != buffers.end())
        it->name = newName;
    else
        Log::Write("Could not find a sound with name \"" + name + "\"", Log::Type::Warning);
}

void SoundManager::RemoveSound(std::string name)
{
    Stop(name);
    auto it = std::find(buffers.begin(), buffers.end(), name);
	if(it != buffers.end())
        buffers.erase(it);
    else
        Log::Write("Could not find a sound with name \"" + name + "\"", Log::Type::Warning);
}

void SoundManager::SetPosition(rp3d::Vector3 pos, std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
    {
        it->pos = pos;
        it->UpdateActiveSound(sounds, id);
    }
}

void SoundManager::SetRelativeToListener(bool relative, std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
    {
        it->relativeToListener = relative;
        it->UpdateActiveSound(sounds, id);
    }
}

void SoundManager::SetLoop(bool loop, std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
    {
        it->loop = loop;
        it->UpdateActiveSound(sounds, id);
    }
}

void SoundManager::SetVolume(float volume, std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
    {
        it->volume = volume;
        it->UpdateActiveSound(sounds, id);
    }
}

void SoundManager::SetMinDistance(float dist, std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
    {
        it->minDistance = dist;
        it->UpdateActiveSound(sounds, id);
    }
}

void SoundManager::SetAttenuation(float attenuation, std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
    {
        it->attenuation = attenuation;
        it->UpdateActiveSound(sounds, id);
    }
}

void SoundManager::SetPitch(float pitch, std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
    {
        it->pitch = pitch;
        it->UpdateActiveSound(sounds, id);
    }
}

rp3d::Vector3 SoundManager::GetPosition(std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
        return it->pos;
    return rp3d::Vector3::zero();
}

bool SoundManager::GetRelativeToListener(std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
        return it->relativeToListener;
    return false;
}

bool SoundManager::GetLoop(std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
        return it->loop;
    return false;
}

float SoundManager::GetVolume(std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
        return it->volume;
    return 0;
}

float SoundManager::GetMinDistance(std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
        return it->minDistance;
    return 0;
}

float SoundManager::GetAttenuation(std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
        return it->attenuation;
    return 0;
}

float SoundManager::GetPitch(std::string name, int id)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
        return it->pitch;
    return 0;
}

void SoundManager::UpdateAll()
{
    std::for_each(buffers.begin(), buffers.end(), [&](auto& b)
    {
        b.UpdateActiveSounds(sounds);
    });
}

void SoundManager::UpdateAll(std::string name)
{
    auto it = std::find(buffers.begin(), buffers.end(), name);
    if(it != buffers.end())
        it->UpdateActiveSounds(sounds);
}

std::vector<std::string> SoundManager::GetSounds()
{
    std::vector<std::string> ret;
    std::for_each(buffers.begin(), buffers.end(), [&](auto& b)
    {
        ret.push_back(b.name);
    });
    return ret;
}

Json::Value SoundManager::Serialize(std::string relativeTo)
{
    Json::Value data;

    for(int i = 0; i < buffers.size(); i++)
    {
        data[i]["name"] = buffers[i].name;
        data[i]["filename"] = std::filesystem::relative(buffers[i].filename, std::filesystem::path(relativeTo).parent_path()).string();

        data[i]["position"]["x"] = buffers[i].pos.x;
        data[i]["position"]["y"] = buffers[i].pos.y;
        data[i]["position"]["z"] = buffers[i].pos.z;

        data[i]["volume"] = buffers[i].volume;
        data[i]["minDistance"] = buffers[i].minDistance;
        data[i]["attenuation"] = buffers[i].attenuation;

        data[i]["loop"] = buffers[i].loop;
        data[i]["relativeToListener"] = buffers[i].relativeToListener;
    }

	return data;
}

void SoundManager::Deserialize(Json::Value data)
{
    int counter = 0;
    while(!data[counter].empty())
    {
        LoadSound(data[counter]["filename"].asString(), data[counter]["name"].asString());

        buffers.back().pos.x = data[counter]["position"]["x"].asFloat();
        buffers.back().pos.y = data[counter]["position"]["y"].asFloat();
        buffers.back().pos.z = data[counter]["position"]["z"].asFloat();

        buffers.back().volume = data[counter]["volume"].asFloat();
        buffers.back().minDistance = data[counter]["minDistance"].asFloat();
        buffers.back().attenuation = data[counter]["attenuation"].asFloat();

        buffers.back().loop = data[counter]["loop"].asBool();
        buffers.back().relativeToListener = data[counter]["relativeToListener"].asBool();

        counter++;
    }
}
