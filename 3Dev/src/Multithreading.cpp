#include <Multithreading.hpp>

Multithreading* Multithreading::instance = nullptr;

Multithreading* Multithreading::GetInstance()
{
    if(!instance)
        instance = new Multithreading;
    return instance;
}

void Multithreading::Update()
{    
    for(auto& i : deleted)
    {
        jobs.erase(jobs.find(i));
        auto it = mainThread.find(i);
        if(it != mainThread.end())
        {
            it->second();
            mainThread.erase(it);
        }
    }

    deleted.clear();

    for(auto& i : jobs)
    {
        if(i.second.wait_for(0ms) == std::future_status::ready)
            deleted.push_back(i.first);
    }
}

void Multithreading::AddJob(std::thread::id threadID, std::future<void>&& future)
{
    jobs[threadID] = std::move(future);
}

void Multithreading::AddMainThreadJob(std::function<void()> job)
{
    std::lock_guard<std::mutex> lock(mtx);

    mainThread[std::this_thread::get_id()] = job;
}
