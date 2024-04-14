#include <Multithreading.hpp>

Multithreading& Multithreading::GetInstance()
{
    static std::unique_ptr<Multithreading> instance(new Multithreading());
    return *instance;
}

void Multithreading::Update()
{
    for(auto& i : deleted)
        jobs.erase(i);

    deleted.clear();

    if(jobs.empty())
        for(auto i = mainThread.begin(); i < mainThread.end(); i++)
        {
            (*i)();
            mainThread.erase(i);
        }

    for(auto i = jobs.begin(); i < jobs.end(); i++)
    {
        if(i->wait_for(0ms) == std::future_status::ready)
            deleted.push_back(i);
    }
}

void Multithreading::AddJob(std::thread::id threadID, std::future<void>&& future)
{
    jobs.emplace_back(std::move(future));
}

void Multithreading::AddMainThreadJob(std::function<void()> job)
{
    mainThread.push_back(job);
}

void Multithreading::SetIsEnabled(bool enabled)
{
    this->enabled = enabled;
}

size_t Multithreading::GetJobsNum()
{
    return jobs.size();
}

bool Multithreading::IsEnabled()
{
    return enabled;
}
