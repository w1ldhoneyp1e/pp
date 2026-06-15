#pragma once

#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <cstddef>
#include <functional>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool
{
public:
    explicit ThreadPool(
        std::size_t threadCount,
        std::size_t queueCapacity = 65534);

    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    template <typename Function>
    void Submit(Function&& function)
    {
        static_assert(
            std::is_invocable_r_v<void, std::decay_t<Function>>,
            "ThreadPool accepts functions callable with no arguments");

        Task* task = new Task(std::forward<Function>(function));

        m_pendingTasks.fetch_add(1, std::memory_order_relaxed);

        while (!m_tasks.push(task))
        {
            std::this_thread::yield();
        }
    }

    void Wait() const noexcept;

    std::size_t GetThreadCount() const noexcept;

    bool IsQueueLockFree() const noexcept;

private:
    using Task = std::function<void()>;

    void WorkerLoop() noexcept;
    void ExecuteTask(Task* task) noexcept;

    boost::lockfree::queue<
        Task*,
        boost::lockfree::fixed_sized<true>>
        m_tasks;

    std::vector<std::thread> m_workers;
    std::atomic<std::size_t> m_pendingTasks;
    std::atomic<bool> m_stopping;
};
