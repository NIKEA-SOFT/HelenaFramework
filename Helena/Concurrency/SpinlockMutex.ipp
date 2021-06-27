#ifndef HELENA_CONCURRENCY_SPINLOCKMUTEX_IPP
#define HELENA_CONCURRENCY_SPINLOCKMUTEX_IPP

#include <Helena/Concurrency/SpinlockMutex.hpp>

#include <atomic>

namespace Helena::Concurrency
{
    SpinlockMutex::SpinlockMutex() : m_Lock{} {}

    void SpinlockMutex::lock() noexcept {
        while(m_Lock.test_and_set(std::memory_order_acquire));
    }

    void SpinlockMutex::unlock() noexcept {
        m_Lock.clear(std::memory_order_release);
    }
}

#endif // HELENA_CONCURRENCY_SPINLOCKMUTEX_IPP
