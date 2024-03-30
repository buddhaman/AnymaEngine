#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>

#include "AnymUtil.h"
#include "Array.h"

// This will not use std::function when i impleented my own. Also all
// c++ multithreading code will be replaced. All news and deletes are temporary. 
// I refuse to do RAII, it is for mediors.
using JobID = U32;
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

    Worker() : thread([this]{ this->Run(); }) {}
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

static Worker* 
CreateWorker(ThreadPool* pool)
{
    Worker* worker = new Worker();
    worker->pool = pool;
    return worker;
}

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

    std::atomic<I32> num_jobs = 0;

    void
    AddJob(std::function<void()>* execute)
    {
        Worker* worker = workers[at_worker];
        at_worker++;
        if(at_worker >= workers.size)
        {
            at_worker = 0;
        }

        Job job;
        job.id = at_id;
        at_id++;
        job.execute = execute;
        worker->queue.Push(job);
        worker->cond_var.notify_one();
        num_jobs++;
    }

    void
    WaitForFinish()
    {
        while(num_jobs > 0)
        {
            // busy wait
        }
    }
};

static void
Execute(ThreadPool* pool, Worker* worker, Job job)
{
    (*job.execute)();
    delete job.execute;
    pool->num_jobs--;
}

static ThreadPool* 
CreateThreadPool(I32 num_workers)
{
    ThreadPool* pool = HeapAlloc(ThreadPool);
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
