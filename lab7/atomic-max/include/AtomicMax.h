#pragma once

#include <atomic>
#include <type_traits>

template <typename T>
class AtomicMax
{
    static_assert(std::is_trivially_copyable_v<T>,
        "AtomicMax requires a trivially copyable type");

public:
    explicit AtomicMax(T value) noexcept
        : m_value(value)
    {
    }

    void Update(T newValue) noexcept
    {
        T currentValue = m_value.load(std::memory_order_relaxed);

        while (currentValue < newValue &&
               !m_value.compare_exchange_weak(
                   currentValue,
                   newValue,
                   std::memory_order_relaxed,
                   std::memory_order_relaxed))
        {
        }
    }

    T GetValue() const noexcept
    {
        return m_value.load(std::memory_order_relaxed);
    }

private:
    std::atomic<T> m_value;
};
