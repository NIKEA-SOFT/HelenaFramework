#ifndef HELENA_CONCURRENCY_SPINLOCKMUTEX_HPP
#define HELENA_CONCURRENCY_SPINLOCKMUTEX_HPP

#include <Helena/Defines.hpp>

#include <atomic>

namespace Helena::Concurrency
{
    class SpinlockMutex final
    {
    public:
        SpinlockMutex();
        ~SpinlockMutex() = default;
        SpinlockMutex(const SpinlockMutex&) = delete;
        SpinlockMutex(SpinlockMutex&&) noexcept = delete;
        SpinlockMutex& operator=(const SpinlockMutex&) = delete;
        SpinlockMutex& operator=(SpinlockMutex&&) noexcept = delete;

        void lock() noexcept;
        void unlock() noexcept;

    private:
        std::atomic_flag m_Lock;
    };
}

#include <Helena/Concurrency/SpinlockMutex.ipp>

#endif // HELENA_CONCURRENCY_SPINLOCKMUTEX_HPP
