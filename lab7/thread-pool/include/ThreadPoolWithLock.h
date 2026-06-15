#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPoolWithLock
{
public:
    explicit ThreadPoolWithLock(std::size_t threadCount)
        : m_pendingTasks(0),
          m_stopping(false)
    {
        if (threadCount == 0)
        {
            throw std::invalid_argument(
                "ThreadPoolWithLock thread count must be greater than zero");
        }

        m_workers.reserve(threadCount);

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

    ~ThreadPoolWithLock()
    {
        Wait();

        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_stopping = true;
        }

        m_taskCondition.notify_all();

        for (std::thread& worker : m_workers)
        {
            worker.join();
        }
    }

    ThreadPoolWithLock(const ThreadPoolWithLock&) = delete;
    ThreadPoolWithLock& operator=(const ThreadPoolWithLock&) = delete;
    ThreadPoolWithLock(ThreadPoolWithLock&&) = delete;
    ThreadPoolWithLock& operator=(ThreadPoolWithLock&&) = delete;

    template <typename Function>
    void Submit(Function&& function)
    {
        static_assert(
            std::is_invocable_r_v<void, std::decay_t<Function>>,
            "ThreadPoolWithLock accepts functions callable with no arguments");

        {
            std::lock_guard<std::mutex> lock(m_queueMutex);

            if (m_stopping)
            {
                throw std::runtime_error(
                    "Cannot submit a task to a stopped thread pool");
            }

            m_tasks.emplace(std::forward<Function>(function));
            m_pendingTasks.fetch_add(1, std::memory_order_relaxed);
        }

        m_taskCondition.notify_one();
    }

    void Wait()
    {
        std::unique_lock<std::mutex> lock(m_waitMutex);

        m_waitCondition.wait(
            lock,
            [this]()
            {
                return m_pendingTasks.load(
                           std::memory_order_acquire) == 0;
            });
    }

    std::size_t GetThreadCount() const noexcept
    {
        return m_workers.size();
    }

private:
    using Task = std::function<void()>;

    void WorkerLoop() noexcept
    {
        while (true)
        {
            Task task;

            {
                std::unique_lock<std::mutex> lock(m_queueMutex);

                m_taskCondition.wait(
                    lock,
                    [this]()
                    {
                        return m_stopping || !m_tasks.empty();
                    });

                if (m_stopping && m_tasks.empty())
                {
                    return;
                }

                task = std::move(m_tasks.front());
                m_tasks.pop();
            }

            try
            {
                task();
            }
            catch (...)
            {
            }

            if (m_pendingTasks.fetch_sub(
                    1,
                    std::memory_order_acq_rel) == 1)
            {
                m_waitCondition.notify_all();
            }
        }
    }

    std::queue<Task> m_tasks;
    std::vector<std::thread> m_workers;

    std::mutex m_queueMutex;
    std::condition_variable m_taskCondition;

    std::mutex m_waitMutex;
    std::condition_variable m_waitCondition;

    std::atomic<std::size_t> m_pendingTasks;
    bool m_stopping;
};
