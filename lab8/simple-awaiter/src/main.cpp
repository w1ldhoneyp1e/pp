#include "MyTask.h"

#include <coroutine>
#include <iostream>

struct MyAwaiter {
    int x;
    int y;

    bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<>) const noexcept
    {
    }

    int await_resume() const noexcept
    {
        return x + y;
    }
};

MyTask CoroutineWithAwait(int x, int y)
{
    std::cout << "Before await\n";
    int result = co_await MyAwaiter { x, y };
    std::cout << result << "\n";
    std::cout << "After await\n";
}

int main()
{
    auto task = CoroutineWithAwait(30, 12);
    std::cout << "Before resume\n";
    task.Resume();
    std::cout << "After resume\n";
    CoroutineWithAwait(5, 10).Resume();
    std::cout << "End of main\n";
}
