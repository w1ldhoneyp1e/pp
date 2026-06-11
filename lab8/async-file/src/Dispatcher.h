#pragma once

#include <coroutine>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <linux/io_uring.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace async_io {

class Dispatcher {
public:
    explicit Dispatcher(unsigned entries = 256)
        : entries_(entries)
    {
        std::memset(&params_, 0, sizeof(params_));

        ringFd_ = static_cast<int>(syscall(__NR_io_uring_setup, entries_, &params_));
        if (ringFd_ < 0) {
            throw std::runtime_error(std::string("io_uring_setup failed: ") + std::strerror(errno));
        }

        const auto sqRingSize = params_.sq_off.array + params_.sq_entries * sizeof(std::uint32_t);
        const auto cqRingSize = params_.cq_off.cqes + params_.cq_entries * sizeof(io_uring_cqe);

        sqRing_ = mmap(nullptr, sqRingSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, ringFd_, IORING_OFF_SQ_RING);
        if (sqRing_ == MAP_FAILED) {
            CloseAfterError("mmap SQ ring failed");
        }

        cqRing_ = mmap(nullptr, cqRingSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, ringFd_, IORING_OFF_CQ_RING);
        if (cqRing_ == MAP_FAILED) {
            CloseAfterError("mmap CQ ring failed");
        }

        sqes_ = static_cast<io_uring_sqe*>(mmap(nullptr,
            params_.sq_entries * sizeof(io_uring_sqe),
            PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_POPULATE,
            ringFd_,
            IORING_OFF_SQES));
        if (sqes_ == MAP_FAILED) {
            CloseAfterError("mmap SQEs failed");
        }

        sqHead_ = Offset<std::uint32_t>(sqRing_, params_.sq_off.head);
        sqTail_ = Offset<std::uint32_t>(sqRing_, params_.sq_off.tail);
        sqRingMask_ = Offset<std::uint32_t>(sqRing_, params_.sq_off.ring_mask);
        sqArray_ = Offset<std::uint32_t>(sqRing_, params_.sq_off.array);

        cqHead_ = Offset<std::uint32_t>(cqRing_, params_.cq_off.head);
        cqTail_ = Offset<std::uint32_t>(cqRing_, params_.cq_off.tail);
        cqRingMask_ = Offset<std::uint32_t>(cqRing_, params_.cq_off.ring_mask);
        cqes_ = Offset<io_uring_cqe>(cqRing_, params_.cq_off.cqes);
    }

    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    ~Dispatcher()
    {
        if (sqes_ && sqes_ != MAP_FAILED) {
            munmap(sqes_, params_.sq_entries * sizeof(io_uring_sqe));
        }
        if (cqRing_ && cqRing_ != MAP_FAILED) {
            const auto cqRingSize = params_.cq_off.cqes + params_.cq_entries * sizeof(io_uring_cqe);
            munmap(cqRing_, cqRingSize);
        }
        if (sqRing_ && sqRing_ != MAP_FAILED) {
            const auto sqRingSize = params_.sq_off.array + params_.sq_entries * sizeof(std::uint32_t);
            munmap(sqRing_, sqRingSize);
        }
        if (ringFd_ >= 0) {
            close(ringFd_);
        }
    }

    struct Operation {
        std::coroutine_handle<> coroutine;
        int result = 0;
    };

    class IoAwaiter {
    public:
        enum class Type {
            Open,
            Read,
            Write,
        };

        IoAwaiter(Dispatcher& dispatcher, std::string path, int flags, mode_t mode)
            : dispatcher_(dispatcher)
            , type_(Type::Open)
            , path_(std::move(path))
            , flags_(flags)
            , mode_(mode)
        {
        }

        IoAwaiter(Dispatcher& dispatcher, Type type, int fd, void* buffer, std::size_t size, std::uint64_t offset)
            : dispatcher_(dispatcher)
            , type_(type)
            , fd_(fd)
            , buffer_(buffer)
            , size_(size)
            , offset_(offset)
        {
        }

        bool await_ready() const noexcept
        {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine)
        {
            operation_.coroutine = coroutine;
            dispatcher_.Submit(*this);
        }

        int await_resume() const
        {
            if (operation_.result < 0) {
                throw std::runtime_error(std::strerror(-operation_.result));
            }
            return operation_.result;
        }

    private:
        friend class Dispatcher;

        Dispatcher& dispatcher_;
        Type type_;
        std::string path_;
        int flags_ = 0;
        mode_t mode_ = 0;
        int fd_ = -1;
        void* buffer_ = nullptr;
        std::size_t size_ = 0;
        std::uint64_t offset_ = 0;
        Operation operation_;
    };

    IoAwaiter Open(std::string path, int flags, mode_t mode)
    {
        return IoAwaiter(*this, std::move(path), flags, mode);
    }

    IoAwaiter Read(int fd, void* buffer, std::size_t size, std::uint64_t offset)
    {
        return IoAwaiter(*this, IoAwaiter::Type::Read, fd, buffer, size, offset);
    }

    IoAwaiter Write(int fd, const void* buffer, std::size_t size, std::uint64_t offset)
    {
        return IoAwaiter(*this, IoAwaiter::Type::Write, fd, const_cast<void*>(buffer), size, offset);
    }

    void Run()
    {
        while (pendingOperations_ > 0) {
            const int rc = static_cast<int>(syscall(__NR_io_uring_enter,
                ringFd_,
                0,
                1,
                IORING_ENTER_GETEVENTS,
                nullptr,
                0));
            if (rc < 0) {
                throw std::runtime_error(std::string("io_uring_enter wait failed: ") + std::strerror(errno));
            }

            ProcessCompletions();
        }
    }

private:
    template <typename T>
    static T* Offset(void* base, std::size_t offset)
    {
        return reinterpret_cast<T*>(static_cast<char*>(base) + offset);
    }

    io_uring_sqe* GetSqe()
    {
        const std::uint32_t head = *sqHead_;
        const std::uint32_t tail = *sqTail_;

        if (tail - head >= params_.sq_entries) {
            throw std::runtime_error("io_uring submission queue is full");
        }

        const std::uint32_t index = tail & *sqRingMask_;
        io_uring_sqe* sqe = &sqes_[index];
        std::memset(sqe, 0, sizeof(*sqe));
        sqArray_[index] = index;
        *sqTail_ = tail + 1;
        return sqe;
    }

    void Submit(IoAwaiter& awaiter)
    {
        io_uring_sqe* sqe = GetSqe();
        sqe->user_data = reinterpret_cast<std::uint64_t>(&awaiter.operation_);

        switch (awaiter.type_) {
        case IoAwaiter::Type::Open:
            sqe->opcode = IORING_OP_OPENAT;
            sqe->fd = AT_FDCWD;
            sqe->addr = reinterpret_cast<std::uint64_t>(awaiter.path_.c_str());
            sqe->open_flags = awaiter.flags_;
            sqe->len = awaiter.mode_;
            break;
        case IoAwaiter::Type::Read:
            sqe->opcode = IORING_OP_READ;
            sqe->fd = awaiter.fd_;
            sqe->addr = reinterpret_cast<std::uint64_t>(awaiter.buffer_);
            sqe->len = static_cast<std::uint32_t>(awaiter.size_);
            sqe->off = awaiter.offset_;
            break;
        case IoAwaiter::Type::Write:
            sqe->opcode = IORING_OP_WRITE;
            sqe->fd = awaiter.fd_;
            sqe->addr = reinterpret_cast<std::uint64_t>(awaiter.buffer_);
            sqe->len = static_cast<std::uint32_t>(awaiter.size_);
            sqe->off = awaiter.offset_;
            break;
        }

        ++pendingOperations_;

        const int rc = static_cast<int>(syscall(__NR_io_uring_enter, ringFd_, 1, 0, 0, nullptr, 0));
        if (rc < 0) {
            --pendingOperations_;
            throw std::runtime_error(std::string("io_uring_enter submit failed: ") + std::strerror(errno));
        }
    }

    void ProcessCompletions()
    {
        std::uint32_t head = *cqHead_;
        const std::uint32_t tail = *cqTail_;

        while (head != tail) {
            io_uring_cqe& cqe = cqes_[head & *cqRingMask_];
            auto* operation = reinterpret_cast<Operation*>(cqe.user_data);
            operation->result = cqe.res;

            ++head;
            *cqHead_ = head;
            --pendingOperations_;

            operation->coroutine.resume();
        }
    }

    [[noreturn]] void CloseAfterError(std::string_view message)
    {
        const std::string error = std::string(message) + ": " + std::strerror(errno);
        if (ringFd_ >= 0) {
            close(ringFd_);
            ringFd_ = -1;
        }
        throw std::runtime_error(error);
    }

    unsigned entries_ = 0;
    int ringFd_ = -1;
    io_uring_params params_ {};

    void* sqRing_ = nullptr;
    void* cqRing_ = nullptr;
    io_uring_sqe* sqes_ = nullptr;

    std::uint32_t* sqHead_ = nullptr;
    std::uint32_t* sqTail_ = nullptr;
    std::uint32_t* sqRingMask_ = nullptr;
    std::uint32_t* sqArray_ = nullptr;

    std::uint32_t* cqHead_ = nullptr;
    std::uint32_t* cqTail_ = nullptr;
    std::uint32_t* cqRingMask_ = nullptr;
    io_uring_cqe* cqes_ = nullptr;

    int pendingOperations_ = 0;
};

} // namespace async_io
