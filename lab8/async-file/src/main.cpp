#include "AsyncFile.h"
#include "Dispatcher.h"
#include "Task.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using async_io::AsyncFile;
using async_io::Dispatcher;
using async_io::OpenMode;
using async_io::Task;
using async_io::AsyncOpenFile;

Task<void> AsyncCopyFile(Dispatcher& dispatcher, std::string from, std::string to)
{
    AsyncFile input = co_await AsyncOpenFile(dispatcher, std::move(from), OpenMode::Read);
    AsyncFile output = co_await AsyncOpenFile(dispatcher, std::move(to), OpenMode::Write);

    std::vector<char> buffer(1024);

    for (unsigned bytesRead = 0;
         (bytesRead = co_await input.ReadAsync(dispatcher, buffer.data(), buffer.size())) != 0;) {
        co_await output.AsyncWrite(dispatcher, buffer.data(), bytesRead);
    }
}

Task<void> AsyncCopyTwoFiles(Dispatcher& dispatcher)
{
    auto t1 = AsyncCopyFile(dispatcher, "a.in", "a.out");
    auto t2 = AsyncCopyFile(dispatcher, "b.in", "b.out");

    co_await t1;
    co_await t2;
}

int main()
{
    {
        std::ofstream("a.in") << "First file\nLine 2\n";
        std::ofstream("b.in") << "Second file\nAnother line\n";
    }

    try {
        Dispatcher dispatcher;
        Task<void> task = AsyncCopyTwoFiles(dispatcher);

        dispatcher.Run();
        task.GetResult();

        std::cout << "Copied a.in -> a.out and b.in -> b.out\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
