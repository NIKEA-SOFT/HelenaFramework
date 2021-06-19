#ifndef HELENA_CONCURRENCY_SPINLOCKMUTEX_HPP
#define HELENA_CONCURRENCY_SPINLOCKMUTEX_HPP

#include <atomic>

namespace Helena::Concurrency
{
    class SpinlockMutex final
    {
    public:
        SpinlockMutex() : m_Lock{} {}
        ~SpinlockMutex() = default;
        SpinlockMutex(const SpinlockMutex&) = delete;
        SpinlockMutex(SpinlockMutex&&) noexcept = delete;
        SpinlockMutex& operator=(const SpinlockMutex&) = delete;
        SpinlockMutex& operator=(SpinlockMutex&&) noexcept = delete;

        void lock() noexcept {
            while(m_Lock.test_and_set(std::memory_order_acquire));
        }

        void unlock() noexcept {
            m_Lock.clear(std::memory_order_release);
        }

    private:
        std::atomic_flag m_Lock;
    };
}

#endif // HELENA_CONCURRENCY_SPINLOCKMUTEX_HPP
