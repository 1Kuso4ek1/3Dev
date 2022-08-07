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
    buffers.emplace_back(filename, name);
}

void SoundManager::LoadSound(sf::SoundBuffer& buffer, std::string name)
{
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
