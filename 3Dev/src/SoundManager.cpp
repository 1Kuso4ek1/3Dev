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
    sf::SoundBuffer buffer;
    buffer.loadFromFile(filename);
    buffers[name] = buffer;
}

void SoundManager::LoadSound(sf::SoundBuffer& buffer, std::string name)
{
    buffers[name] = buffer;
}

void SoundManager::Play(std::string name, int id, rp3d::Vector3 pos, bool relativeToListener)
{
    sounds.push_back({ name + std::to_string(id), sf::Sound(buffers[name]) });
    sounds.back().second.setPosition(pos.x, pos.y, pos.z);
    sounds.back().second.setRelativeToListener(relativeToListener);
    sounds.back().second.play();
}

void SoundManager::Stop(std::string name, int id)
{
    auto it = std::find_if(sounds.begin(), sounds.end(), [&](auto& s) { return s.first == name + std::to_string(id); });
    if(it != sounds.end())
    {
        it->second.stop();
        sounds.erase(it);
    }
}

void SoundManager::Pause(std::string name, int id)
{
    auto it = std::find_if(sounds.begin(), sounds.end(), [&](auto& s) { return s.first == name + std::to_string(id); });
    if(it != sounds.end())
        it->second.pause();
}

void SoundManager::SetPosition(rp3d::Vector3 pos, std::string name, int id)
{
    auto it = std::find_if(sounds.begin(), sounds.end(), [&](auto& s) { return s.first == name + std::to_string(id); });
    if(it != sounds.end())
        it->second.setPosition(pos.x, pos.y, pos.z);
}

void SoundManager::SetRelativeToListener(bool relative, std::string name, int id)
{
    auto it = std::find_if(sounds.begin(), sounds.end(), [&](auto& s) { return s.first == name + std::to_string(id); });
    if(it != sounds.end())
        it->second.setRelativeToListener(relative);
}

void SoundManager::SetLoop(bool loop, std::string name, int id)
{
    auto it = std::find_if(sounds.begin(), sounds.end(), [&](auto& s) { return s.first == name + std::to_string(id); });
    if(it != sounds.end())
        it->second.setLoop(loop);
}

void SoundManager::SetVolume(float volume, std::string name, int id)
{
    auto it = std::find_if(sounds.begin(), sounds.end(), [&](auto& s) { return s.first == name + std::to_string(id); });
    if(it != sounds.end())
        it->second.setVolume(volume);
}

void SoundManager::SetMinDistance(float dist, std::string name, int id)
{
    auto it = std::find_if(sounds.begin(), sounds.end(), [&](auto& s) { return s.first == name + std::to_string(id); });
    if(it != sounds.end())
        it->second.setMinDistance(dist);
}

void SoundManager::SetAttenuation(float attenuation, std::string name, int id)
{
    auto it = std::find_if(sounds.begin(), sounds.end(), [&](auto& s) { return s.first == name + std::to_string(id); });
    if(it != sounds.end())
        it->second.setAttenuation(attenuation);
}

void SoundManager::Cleanup()
{
    for(auto i = sounds.begin(); i < sounds.end(); i++)
        if(i->second.getStatus() == sf::Sound::Status::Stopped)
            sounds.erase(i);
}
