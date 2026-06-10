#include "MyTask.h"

#include <iostream>

MyTask SimpleCoroutine()
{
    co_return "Hello from coroutine!";
}

int main()
{
    MyTask task = SimpleCoroutine();
    std::cout << task.GetResult() << std::endl;

    return 0;
}
