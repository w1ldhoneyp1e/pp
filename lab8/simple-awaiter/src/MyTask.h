#pragma once

#include <coroutine>
#include <exception>
#include <stdexcept>
#include <utility>

class MyTask {
public:
    struct promise_type {
        std::exception_ptr exception;

        MyTask get_return_object()
        {
            return MyTask(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        std::suspend_never initial_suspend() noexcept
        {
            return {};
        }

        std::suspend_always final_suspend() noexcept
        {
            return {};
        }

        void return_void() noexcept
        {
        }

        void unhandled_exception() noexcept
        {
            exception = std::current_exception();
        }
    };

    MyTask(const MyTask&) = delete;
    MyTask& operator=(const MyTask&) = delete;

    MyTask(MyTask&& other) noexcept
        : coroutine_(std::exchange(other.coroutine_, nullptr))
    {
    }

    MyTask& operator=(MyTask&& other) noexcept
    {
        if (this != &other) {
            Destroy();
            coroutine_ = std::exchange(other.coroutine_, nullptr);
        }
        return *this;
    }

    ~MyTask()
    {
        Destroy();
    }

    void Resume()
    {
        if (!coroutine_) {
            throw std::runtime_error("MyTask does not contain a coroutine");
        }

        if (!coroutine_.done()) {
            coroutine_.resume();
        }

        if (coroutine_.promise().exception) {
            std::rethrow_exception(coroutine_.promise().exception);
        }
    }

private:
    explicit MyTask(std::coroutine_handle<promise_type> coroutine)
        : coroutine_(coroutine)
    {
    }

    void Destroy() noexcept
    {
        if (coroutine_) {
            coroutine_.destroy();
            coroutine_ = nullptr;
        }
    }

    std::coroutine_handle<promise_type> coroutine_;
};
