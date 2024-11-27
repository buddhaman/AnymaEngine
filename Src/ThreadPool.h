#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>

#include "AnymUtil.h"
#include "Array.h"

#include "windows.h"

// This will not use std::function when i impleented my own. Also all
// c++ multithreading code will be replaced. 
struct JobID { U32 id; };
struct Job
{
    JobID id;
    std::function<void()>* execute;
};

struct JobQueue
{

    DynamicArray<Job> jobs;
    U32 queue_at = 0;

    bool
    Empty()
    {
        return queue_at==jobs.size;
    }

    void
    Push(Job job)
    {
        jobs.PushBack(job);
    }

    Job
    Pop()
    {
        Assert(!Empty());
        Job job = jobs[queue_at];
        queue_at++;
        const int to_remove = 256;
        if(queue_at > to_remove)
        {
            jobs.RemoveRange(0, queue_at - 1);
            queue_at = 0;
        }
        return job;
    }
};

struct ThreadPool;
struct Worker;

static void
Execute(ThreadPool* pool, Worker* worker, Job job);

struct Worker
{
    std::condition_variable cond_var;
    std::mutex queue_mutex;
    JobQueue queue;
    std::thread thread;
    bool running = true;
    ThreadPool* pool;

    Worker(int id) : thread([this]{ this->Run(); }) 
    {
        std::string name = "AnymaWorker"+std::to_string(id);
        SetThreadDescription(thread.native_handle(), std::wstring(name.begin(), name.end()).c_str());
    }

    ~Worker() 
    { 
        if (thread.joinable()) 
        { 
            thread.join(); 
        }
    }

    void Run()
    {
        while(running)
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cond_var.wait(lock, [this] { return !queue.Empty() || !running; });
            if(!running)
            {
                break;
            }

            Job job = queue.Pop();
            Execute(pool, this, job);
        }
    }
};

Worker* 
CreateWorker(ThreadPool* pool);

static void
DestroyWorker(Worker* worker)
{
    delete worker;
}

struct ThreadPool
{
    U32 at_id;
    I32 at_worker;  // For round robin assignment.
    Array<Worker*> workers;

    std::condition_variable jobs_done_condition;
    std::mutex jobs_done_mutex;
    I32 num_jobs = 0;

    void AddJob(std::function<void()>* execute)
    {
        std::unique_lock<std::mutex> lock(jobs_done_mutex);
        num_jobs++;
        Worker* worker = workers[at_worker];
        at_worker = (at_worker + 1) % workers.size;

        Job job;
        job.id.id = at_id++;
        job.execute = execute;

        {
            std::lock_guard<std::mutex> queue_lock(worker->queue_mutex);
            worker->queue.Push(job);
        }
        worker->cond_var.notify_one();
    }

    void
    StartAndWaitForFinish()
    {
        for(Worker* worker : workers)
        {
            worker->cond_var.notify_all();
        }

        std::unique_lock lock(jobs_done_mutex);
        jobs_done_condition.wait(lock, [this](){ return num_jobs==0; });
    }
};

static void 
Execute(ThreadPool* pool, Worker* worker, Job job)
{
    (*job.execute)();
    delete job.execute;

    {
        std::unique_lock<std::mutex> lock(pool->jobs_done_mutex);
        pool->num_jobs--;
        if(pool->num_jobs == 0)
        {
            pool->jobs_done_condition.notify_all();
        }
    }
}

static ThreadPool* 
CreateThreadPool(I32 num_workers)
{
    ThreadPool* pool = new (HeapAlloc(ThreadPool))ThreadPool();
    pool->at_id = 1;
    pool->workers = CreateArray<Worker*>(num_workers);
    for(int i = 0; i < num_workers; i++)
    {
        pool->workers.PushBack(CreateWorker(pool));
    }
    return pool;
}

static void 
DestroyThreadPool(ThreadPool* pool)
{
    for(Worker* worker : pool->workers)
    {
        worker->running = false;
        worker->cond_var.notify_one();
        DestroyWorker(worker);
    }
}

Worker* 
CreateWorker(ThreadPool* pool)
{
    Worker* worker = new Worker((int)pool->workers.size);
    worker->pool = pool;
    return worker;
}
