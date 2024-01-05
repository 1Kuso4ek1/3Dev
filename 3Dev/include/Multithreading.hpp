#pragma once
#include "Utils.hpp"

using namespace std::chrono_literals;

class Multithreading
{
public:
    static Multithreading* GetInstance();

    void Update();

    void AddJob(std::thread::id threadID, std::future<void>&& future);
    
    void AddMainThreadJob(std::function<void()> job);

private:
    Multithreading() {};

    static Multithreading* instance;

    /*std::unordered_map<std::thread::id, std::future<void>> jobs;
    std::unordered_map<std::thread::id, std::function<void()>> mainThread;*/

    std::vector<std::future<void>> jobs;
    std::vector<std::function<void()>> mainThread;

    std::vector<std::vector<std::future<void>>::iterator> deleted;
};
