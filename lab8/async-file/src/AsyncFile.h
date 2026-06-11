#pragma once

#include "Dispatcher.h"
#include "Task.h"

#include <cstdint>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace async_io {

enum class OpenMode {
    Read,
    Write,
};

class AsyncFile {
public:
    AsyncFile() = default;

    explicit AsyncFile(int fd)
        : fd_(fd)
    {
    }

    AsyncFile(const AsyncFile&) = delete;
    AsyncFile& operator=(const AsyncFile&) = delete;

    AsyncFile(AsyncFile&& other) noexcept
        : fd_(std::exchange(other.fd_, -1))
        , offset_(std::exchange(other.offset_, 0))
    {
    }

    AsyncFile& operator=(AsyncFile&& other) noexcept
    {
        if (this != &other) {
            Close();
            fd_ = std::exchange(other.fd_, -1);
            offset_ = std::exchange(other.offset_, 0);
        }
        return *this;
    }

    ~AsyncFile()
    {
        Close();
    }

    Task<unsigned> ReadAsync(Dispatcher& dispatcher, char* buffer, std::size_t size)
    {
        CheckOpened();
        const int bytesRead = co_await dispatcher.Read(fd_, buffer, size, offset_);
        offset_ += static_cast<std::uint64_t>(bytesRead);
        co_return static_cast<unsigned>(bytesRead);
    }

    Task<void> AsyncWrite(Dispatcher& dispatcher, const char* buffer, std::size_t size)
    {
        CheckOpened();

        std::size_t written = 0;
        while (written < size) {
            const int part = co_await dispatcher.Write(fd_, buffer + written, size - written, offset_);
            if (part == 0) {
                throw std::runtime_error("write returned 0 bytes");
            }
            written += static_cast<std::size_t>(part);
            offset_ += static_cast<std::uint64_t>(part);
        }
    }

private:
    void CheckOpened() const
    {
        if (fd_ < 0) {
            throw std::runtime_error("file is not opened");
        }
    }

    void Close() noexcept
    {
        if (fd_ >= 0) {
            close(fd_);
            fd_ = -1;
        }
    }

    int fd_ = -1;
    std::uint64_t offset_ = 0;
};

Task<AsyncFile> AsyncOpenFile(Dispatcher& dispatcher, std::string path, OpenMode mode)
{
    int flags = 0;
    mode_t permissions = 0644;

    switch (mode) {
    case OpenMode::Read:
        flags = O_RDONLY;
        break;
    case OpenMode::Write:
        flags = O_WRONLY | O_CREAT | O_TRUNC;
        break;
    }

    const int fd = co_await dispatcher.Open(std::move(path), flags, permissions);
    co_return AsyncFile(fd);
}

} // namespace async_io
