#pragma once
#include "3Dev.hpp"
#include <SFML/Audio.hpp>

class ListenerWrapper
{
public:
    static void SetPosition(rp3d::Vector3 pos);
    static void SetUpVector(rp3d::Vector3 vec);
    static void SetOrientation(rp3d::Quaternion orient);
    static void SetGlobalVolume(float volume);

private:
    ListenerWrapper();
};

class SoundManager
{
public:
    void LoadSound(std::string filename, std::string name);
    void LoadSound(sf::SoundBuffer& buffer, std::string name);

    void Play(std::string name, int id = 0);
    void PlayAt(std::string name, int id = 0, rp3d::Vector3 pos = { 0, 0, 0 });
    void Stop(std::string name, int id = 0);
    void Pause(std::string name, int id = 0);

    void SetPosition(rp3d::Vector3 pos, std::string name, int id = 0);
    void SetRelativeToListener(bool relative, std::string name, int id = 0);
    void SetLoop(bool loop, std::string name, int id = 0);
    void SetVolume(float volume, std::string name, int id = 0);
    void SetMinDistance(float dist, std::string name, int id = 0);
    void SetAttenuation(float attenuation, std::string name, int id = 0);

    void UpdateAll();
    void UpdateAll(std::string name);

private:

    struct Sound
    {
        Sound(std::string filename, std::string name) : name(name), filename(filename)
        {
            if(buffer.loadFromFile(filename))
                Log::Write("Sound \"" + filename + "\" successfully loaded", Log::Type::Info);
            else Log::Write("Failed to load sound \"" + filename + "\"", Log::Type::Error);
        }

        Sound(sf::SoundBuffer buffer, std::string name) : name(name), buffer(buffer) {}

        std::string name, filename;

        float volume = 100, minDistance = 0, attenuation = 1;
        bool loop = false, relativeToListener = false;
        
        rp3d::Vector3 pos;

        sf::SoundBuffer buffer;

        void UpdateActiveSound(std::unordered_map<std::string, std::shared_ptr<sf::Sound>>& sounds, int id)
        {
            auto it = sounds.find(name + std::to_string(id));
            if(it != sounds.end())
            {
                it->second->setVolume(volume);
                it->second->setMinDistance(minDistance);
                it->second->setAttenuation(attenuation);
                it->second->setLoop(loop);
                it->second->setRelativeToListener(relativeToListener);
                it->second->setPosition(pos.x, pos.y, pos.z);
            }
        }

        void UpdateActiveSounds(std::unordered_map<std::string, std::shared_ptr<sf::Sound>>& sounds)
        {
            auto it = sounds.begin();
            while(it != sounds.end())
            {
                it = std::find_if(sounds.begin(), sounds.end(), [&](auto& s)
                {
                    return s.first.find(name) != std::string::npos;
                });
                if(it != sounds.end())
                {
                    it->second->setVolume(volume);
                    it->second->setMinDistance(minDistance);
                    it->second->setAttenuation(attenuation);
                    it->second->setLoop(loop);
                    it->second->setRelativeToListener(relativeToListener);
                    it->second->setPosition(pos.x, pos.y, pos.z);
                }
            }
        }

        bool operator==(std::string r)
        {
            return name == r;
        }
    };

    std::vector<Sound> buffers;
    std::unordered_map<std::string, std::shared_ptr<sf::Sound>> sounds;
};
