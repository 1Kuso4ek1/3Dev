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

    /*
    TODO:
    Add own sound and sound buffer structures
    Make Pause useful
    */

    void LoadSound(std::string filename, std::string name);
    void LoadSound(sf::SoundBuffer& buffer, std::string name);

    void Play(std::string name, int id = 0, rp3d::Vector3 pos = { 0, 0, 0 }, bool relativeToListener = true);
    void Stop(std::string name, int id = 0);
    void Pause(std::string name, int id = 0); // not really useful at the time

    void SetPosition(rp3d::Vector3 pos, std::string name, int id = 0);
    void SetRelativeToListener(bool relative, std::string name, int id = 0);
    void SetLoop(bool loop, std::string name, int id = 0);
    void SetVolume(float volume, std::string name, int id = 0);
    void SetMinDistance(float dist, std::string name, int id = 0);
    void SetAttenuation(float attenuation, std::string name, int id = 0);

    void Cleanup();

private:
    std::unordered_map<std::string, sf::SoundBuffer> buffers;
    std::vector<std::pair<std::string, sf::Sound>> sounds;
};
