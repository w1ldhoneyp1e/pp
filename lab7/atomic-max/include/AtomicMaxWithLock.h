#pragma once

#include <mutex>

template <typename T>
class AtomicMaxWithLock
{
public:
    explicit AtomicMaxWithLock(T value) noexcept
        : m_value(value)
    {
    }

    void Update(T newValue) noexcept
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_value < newValue)
        {
            m_value = newValue;
        }
    }

    T GetValue() const noexcept
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_value;
    }

private:
    T m_value;
    mutable std::mutex m_mutex;
};
