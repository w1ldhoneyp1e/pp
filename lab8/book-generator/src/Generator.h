#pragma once

#include <coroutine>
#include <exception>
#include <memory>
#include <utility>

template <typename T>
class Generator {
public:
    struct promise_type {
        T currentValue;
        std::exception_ptr exception;

        Generator get_return_object()
        {
            return Generator(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        std::suspend_always initial_suspend() noexcept
        {
            return {};
        }

        std::suspend_always final_suspend() noexcept
        {
            return {};
        }

        std::suspend_always yield_value(T value) noexcept
        {
            currentValue = std::move(value);
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

    class iterator {
    public:
        using coroutine_handle = std::coroutine_handle<promise_type>;

        iterator() = default;

        explicit iterator(coroutine_handle coroutine)
            : coroutine_(coroutine)
        {
        }

        iterator& operator++()
        {
            coroutine_.resume();
            if (coroutine_.done()) {
                CheckException();
                coroutine_ = nullptr;
            }
            return *this;
        }

        const T& operator*() const
        {
            return coroutine_.promise().currentValue;
        }

        const T* operator->() const
        {
            return std::addressof(operator*());
        }

        bool operator==(std::default_sentinel_t) const noexcept
        {
            return coroutine_ == nullptr;
        }

    private:
        void CheckException() const
        {
            if (coroutine_.promise().exception) {
                std::rethrow_exception(coroutine_.promise().exception);
            }
        }

        coroutine_handle coroutine_ = nullptr;
    };

    using coroutine_handle = std::coroutine_handle<promise_type>;

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    Generator(Generator&& other) noexcept
        : coroutine_(std::exchange(other.coroutine_, nullptr))
    {
    }

    Generator& operator=(Generator&& other) noexcept
    {
        if (this != &other) {
            Destroy();
            coroutine_ = std::exchange(other.coroutine_, nullptr);
        }
        return *this;
    }

    ~Generator()
    {
        Destroy();
    }

    iterator begin()
    {
        if (!coroutine_) {
            return iterator {};
        }

        coroutine_.resume();
        if (coroutine_.done()) {
            if (coroutine_.promise().exception) {
                std::rethrow_exception(coroutine_.promise().exception);
            }
            return iterator {};
        }

        return iterator(coroutine_);
    }

    std::default_sentinel_t end() noexcept
    {
        return {};
    }

private:
    explicit Generator(coroutine_handle coroutine)
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

    coroutine_handle coroutine_;
};
