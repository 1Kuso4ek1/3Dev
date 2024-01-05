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
    void SetIsEnabled(bool enabled);

    size_t GetJobsNum();
    bool IsEnabled();

private:
    Multithreading() {};

    static Multithreading* instance;

    bool enabled = true;

    std::vector<std::future<void>> jobs;
    std::vector<std::function<void()>> mainThread;

    std::vector<std::vector<std::future<void>>::iterator> deleted;
};
