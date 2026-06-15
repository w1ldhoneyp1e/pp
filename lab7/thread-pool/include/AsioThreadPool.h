#pragma once

#include <atomic>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <cstddef>
#include <thread>
#include <type_traits>
#include <utility>

class AsioThreadPool
{
public:
    explicit AsioThreadPool(std::size_t threadCount)
        : m_pool(threadCount),
          m_pendingTasks(0),
          m_threadCount(threadCount)
    {
    }

    ~AsioThreadPool()
    {
        Wait();
        m_pool.join();
    }

    AsioThreadPool(const AsioThreadPool&) = delete;
    AsioThreadPool& operator=(const AsioThreadPool&) = delete;
    AsioThreadPool(AsioThreadPool&&) = delete;
    AsioThreadPool& operator=(AsioThreadPool&&) = delete;

    template <typename Function>
    void Submit(Function&& function)
    {
        static_assert(
            std::is_invocable_r_v<void, std::decay_t<Function>>,
            "AsioThreadPool accepts functions callable with no arguments");

        m_pendingTasks.fetch_add(1, std::memory_order_relaxed);

        boost::asio::post(
            m_pool,
            [this, task = std::forward<Function>(function)]() mutable
            {
                try
                {
                    task();
                }
                catch (...)
                {
                }

                m_pendingTasks.fetch_sub(
                    1,
                    std::memory_order_acq_rel);
            });
    }

    void Wait() const noexcept
    {
        while (m_pendingTasks.load(std::memory_order_acquire) != 0)
        {
            std::this_thread::yield();
        }
    }

    std::size_t GetThreadCount() const noexcept
    {
        return m_threadCount;
    }

private:
    boost::asio::thread_pool m_pool;
    std::atomic<std::size_t> m_pendingTasks;
    std::size_t m_threadCount;
};
