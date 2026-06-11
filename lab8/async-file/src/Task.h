#pragma once

#include <coroutine>
#include <exception>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace async_io {

namespace detail {

struct FinalAwaiter {
    bool await_ready() const noexcept
    {
        return false;
    }

    template <typename Promise>
    std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> coroutine) const noexcept
    {
        auto continuation = coroutine.promise().continuation;
        return continuation ? continuation : std::noop_coroutine();
    }

    void await_resume() const noexcept
    {
    }
};

} // namespace detail

template <typename T = void>
class Task {
public:
    struct promise_type {
        std::optional<T> result;
        std::exception_ptr exception;
        std::coroutine_handle<> continuation = nullptr;

        Task get_return_object()
        {
            return Task(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        std::suspend_never initial_suspend() noexcept
        {
            return {};
        }

        detail::FinalAwaiter final_suspend() noexcept
        {
            return {};
        }

        template <typename U>
            requires std::is_convertible_v<U, T>
        void return_value(U&& value)
        {
            result.emplace(std::forward<U>(value));
        }

        void unhandled_exception() noexcept
        {
            exception = std::current_exception();
        }
    };

    using coroutine_handle = std::coroutine_handle<promise_type>;

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other) noexcept
        : coroutine_(std::exchange(other.coroutine_, nullptr))
    {
    }

    Task& operator=(Task&& other) noexcept
    {
        if (this != &other) {
            Destroy();
            coroutine_ = std::exchange(other.coroutine_, nullptr);
        }
        return *this;
    }

    ~Task()
    {
        Destroy();
    }

    bool IsDone() const noexcept
    {
        return !coroutine_ || coroutine_.done();
    }

    T GetResult()
    {
        if (!coroutine_) {
            throw std::runtime_error("Task does not contain a coroutine");
        }
        if (!coroutine_.done()) {
            throw std::runtime_error("Task is not completed yet; call Dispatcher::Run() first");
        }
        return ExtractResult(coroutine_);
    }

    struct Awaiter {
        coroutine_handle coroutine;

        bool await_ready() const noexcept
        {
            return !coroutine || coroutine.done();
        }

        void await_suspend(std::coroutine_handle<> continuation) const noexcept
        {
            coroutine.promise().continuation = continuation;
        }

        T await_resume() const
        {
            return Task::ExtractResult(coroutine);
        }
    };

    Awaiter operator co_await() & noexcept
    {
        return Awaiter { coroutine_ };
    }

    Awaiter operator co_await() && noexcept
    {
        return Awaiter { coroutine_ };
    }

private:
    explicit Task(coroutine_handle coroutine)
        : coroutine_(coroutine)
    {
    }

    static T ExtractResult(coroutine_handle coroutine)
    {
        auto& promise = coroutine.promise();
        if (promise.exception) {
            std::rethrow_exception(promise.exception);
        }
        if (!promise.result) {
            throw std::runtime_error("Task finished without result");
        }
        return std::move(*promise.result);
    }

    void Destroy() noexcept
    {
        if (coroutine_) {
            coroutine_.destroy();
            coroutine_ = nullptr;
        }
    }

    coroutine_handle coroutine_ = nullptr;
};

template <>
class Task<void> {
public:
    struct promise_type {
        std::exception_ptr exception;
        std::coroutine_handle<> continuation = nullptr;

        Task get_return_object()
        {
            return Task(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        std::suspend_never initial_suspend() noexcept
        {
            return {};
        }

        detail::FinalAwaiter final_suspend() noexcept
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

    using coroutine_handle = std::coroutine_handle<promise_type>;

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other) noexcept
        : coroutine_(std::exchange(other.coroutine_, nullptr))
    {
    }

    Task& operator=(Task&& other) noexcept
    {
        if (this != &other) {
            Destroy();
            coroutine_ = std::exchange(other.coroutine_, nullptr);
        }
        return *this;
    }

    ~Task()
    {
        Destroy();
    }

    bool IsDone() const noexcept
    {
        return !coroutine_ || coroutine_.done();
    }

    void GetResult()
    {
        if (!coroutine_) {
            throw std::runtime_error("Task does not contain a coroutine");
        }
        if (!coroutine_.done()) {
            throw std::runtime_error("Task is not completed yet; call Dispatcher::Run() first");
        }
        ExtractResult(coroutine_);
    }

    struct Awaiter {
        coroutine_handle coroutine;

        bool await_ready() const noexcept
        {
            return !coroutine || coroutine.done();
        }

        void await_suspend(std::coroutine_handle<> continuation) const noexcept
        {
            coroutine.promise().continuation = continuation;
        }

        void await_resume() const
        {
            Task::ExtractResult(coroutine);
        }
    };

    Awaiter operator co_await() & noexcept
    {
        return Awaiter { coroutine_ };
    }

    Awaiter operator co_await() && noexcept
    {
        return Awaiter { coroutine_ };
    }

private:
    explicit Task(coroutine_handle coroutine)
        : coroutine_(coroutine)
    {
    }

    static void ExtractResult(coroutine_handle coroutine)
    {
        if (coroutine.promise().exception) {
            std::rethrow_exception(coroutine.promise().exception);
        }
    }

    void Destroy() noexcept
    {
        if (coroutine_) {
            coroutine_.destroy();
            coroutine_ = nullptr;
        }
    }

    coroutine_handle coroutine_ = nullptr;
};

} // namespace async_io
