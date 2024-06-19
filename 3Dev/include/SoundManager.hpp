#pragma once
#include "Utils.hpp"
#include <SFML/Audio.hpp>

class ListenerWrapper
{
public:
    static void SetPosition(const rp3d::Vector3& pos);
    static void SetUpVector(const rp3d::Vector3& vec);
    static void SetOrientation(const rp3d::Quaternion& orient);
    static void SetGlobalVolume(float volume);

private:
    static rp3d::Vector3 prevPos;

    ListenerWrapper();
};

class SoundManager
{
public:
    void LoadSound(const std::string& filename, std::string name = "sound", bool checkUniqueness = true);
    void LoadSound(sf::SoundBuffer& buffer, std::string name = "sound", bool checkUniqueness = true);

    void Play(const std::string& name, int id = 0);
    void PlayAt(const std::string& name, int id = 0, const rp3d::Vector3& pos = { 0, 0, 0 });
    void PlayMono(const std::string& name, int id = 0);
    void Stop(const std::string& name, int id = 0);
    void Pause(const std::string& name, int id = 0);

    void SetName(const std::string& name, const std::string& newName);
    void RemoveSound(const std::string& name);

    void SetPosition(const rp3d::Vector3& pos, const std::string& name, int id = 0);
    void SetRelativeToListener(bool relative, const std::string& name, int id = 0);
    void SetLoop(bool loop, const std::string& name, int id = 0);
    void SetVolume(float volume, const std::string& name, int id = 0);
    void SetMinDistance(float dist, const std::string& name, int id = 0);
    void SetAttenuation(float attenuation, const std::string& name, int id = 0);
    void SetPitch(float pitch, const std::string& name, int id = 0);
    void SetDopplerFactor(float dopplerFactor, const std::string& name, int id = 0);

    rp3d::Vector3 GetPosition(const std::string& name, int id = 0);
    bool GetRelativeToListener(const std::string& name, int id = 0);
    bool GetLoop(const std::string& name, int id = 0);
    float GetVolume(const std::string& name, int id = 0);
    float GetMinDistance(const std::string& name, int id = 0);
    float GetAttenuation(const std::string& name, int id = 0);
    float GetPitch(const std::string& name, int id = 0);
    float GetDopplerFactor(const std::string& name, int id = 0);

    void UpdateAll();
    void UpdateAll(const std::string& name);

    std::vector<std::string> GetSounds();

    Json::Value Serialize(const std::string& relativeTo);
    void Deserialize(Json::Value data);

private:

    struct Sound
    {
        Sound(const std::string& filename, std::string name) : name(name), filename(filename)
        {
            if(buffer.loadFromFile(filename))
                Log::Write("Sound \"" + filename + "\" successfully loaded", Log::Type::Info);
            else Log::Write("Failed to load sound \"" + filename + "\"", Log::Type::Error);
        }

        Sound(sf::SoundBuffer buffer, std::string name) : name(name), buffer(buffer) {}

        std::string name, filename;

        float volume = 100, minDistance = 1, attenuation = 0, pitch = 1, dopplerFactor = 0;
        bool loop = false, relativeToListener = false;
        
        rp3d::Vector3 pos, prevPos;

        sf::SoundBuffer buffer;

        void UpdateActiveSound(std::unordered_map<std::string, std::shared_ptr<sf::Sound>>& sounds, int id)
        {
            auto it = sounds.find(name + std::to_string(id));
            if(it != sounds.end())
            {
                if(minDistance == 0)
                    it->second->setSpatializationEnabled(false);
                else
                    it->second->setSpatializationEnabled(true);
                it->second->setDopplerFactor(dopplerFactor);
                it->second->setVolume(volume);
                it->second->setMinDistance(minDistance);
                it->second->setAttenuation(attenuation);
                it->second->setPitch(pitch);
                it->second->setLoop(loop);
                it->second->setRelativeToListener(relativeToListener);

                auto velocity = pos - prevPos;

                it->second->setPosition({ pos.x, pos.y, pos.z });
                it->second->setVelocity({ velocity.x, velocity.y, velocity.z });

                prevPos = pos;
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
                    it->second->setPitch(pitch);
                    it->second->setLoop(loop);
                    it->second->setRelativeToListener(relativeToListener);
                    it->second->setPosition({ pos.x, pos.y, pos.z });
                }
            }
        }

        rp3d::Vector3 GetPosition(std::unordered_map<std::string, std::shared_ptr<sf::Sound>>& sounds, int id)
        {
            auto it = sounds.find(name + std::to_string(id));
            if(it != sounds.end())
            {
                auto v = it->second->getPosition();
                return { v.x, v.y, v.z };
            }

            return pos;
        }

        /*bool GetRelativeToListener(std::unordered_map<std::string, std::shared_ptr<sf::Sound>>& sounds, int id);
        bool GetLoop(std::unordered_map<std::string, std::shared_ptr<sf::Sound>>& sounds, int id);
        float GetVolume(std::unordered_map<std::string, std::shared_ptr<sf::Sound>>& sounds, int id);
        float GetMinDistance(std::unordered_map<std::string, std::shared_ptr<sf::Sound>>& sounds, int id);
        float GetAttenuation(std::unordered_map<std::string, std::shared_ptr<sf::Sound>>& sounds, int id);
        float GetPitch(std::unordered_map<std::string, std::shared_ptr<sf::Sound>>& sounds, int id);*/

        bool operator==(std::string r)
        {
            return name == r;
        }
    };

    std::vector<Sound> buffers;
    std::unordered_map<std::string, std::shared_ptr<sf::Sound>> sounds;
};
