#include "../include/ThreadPool.h"

#include <exception>
#include <stdexcept>

ThreadPool::ThreadPool(
    std::size_t threadCount,
    std::size_t queueCapacity)
    : m_tasks(queueCapacity),
      m_pendingTasks(0),
      m_stopping(false)
{
    if (threadCount == 0)
    {
        throw std::invalid_argument(
            "ThreadPool thread count must be greater than zero");
    }

    if (queueCapacity == 0)
    {
        throw std::invalid_argument(
            "ThreadPool queue capacity must be greater than zero");
    }

    m_workers.reserve(threadCount);

    try
    {
        for (std::size_t threadIndex = 0;
             threadIndex < threadCount;
             ++threadIndex)
        {
            m_workers.emplace_back(
                [this]()
                {
                    WorkerLoop();
                });
        }
    }
    catch (...)
    {
        m_stopping.store(true, std::memory_order_release);

        for (std::thread& worker : m_workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }

        throw;
    }
}

ThreadPool::~ThreadPool()
{
    Wait();
    m_stopping.store(true, std::memory_order_release);

    for (std::thread& worker : m_workers)
    {
        worker.join();
    }

    Task* task = nullptr;

    while (m_tasks.pop(task))
    {
        delete task;
    }
}

void ThreadPool::Wait() const noexcept
{
    while (m_pendingTasks.load(std::memory_order_acquire) != 0)
    {
        std::this_thread::yield();
    }
}

std::size_t ThreadPool::GetThreadCount() const noexcept
{
    return m_workers.size();
}

bool ThreadPool::IsQueueLockFree() const noexcept
{
    return m_tasks.is_lock_free();
}

void ThreadPool::WorkerLoop() noexcept
{
    while (true)
    {
        Task* task = nullptr;

        if (m_tasks.pop(task))
        {
            ExecuteTask(task);
            continue;
        }

        if (m_stopping.load(std::memory_order_acquire))
        {
            break;
        }

        std::this_thread::yield();
    }
}

void ThreadPool::ExecuteTask(Task* task) noexcept
{
    try
    {
        (*task)();
    }
    catch (...)
    {
    }

    delete task;
    m_pendingTasks.fetch_sub(1, std::memory_order_acq_rel);
}
