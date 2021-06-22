#ifndef HELENA_CONCURRENCY_SPINLOCK_IPP
#define HELENA_CONCURRENCY_SPINLOCK_IPP

#include <Helena/Concurrency/Spinlock.hpp>

namespace Helena::Concurrency {

    Spinlock::Spinlock() : m_Lock {} {}

    void Spinlock::lock() noexcept
    {
        for (;;) {
            if (!m_Lock.exchange(true, std::memory_order_acquire)) {
                return;
            }

            // todo: add sleep if long time spined
            while (m_Lock.load(std::memory_order_relaxed)) {}
        }
    }

    [[nodiscard]] bool Spinlock::try_lock() noexcept {
        return !m_Lock.load(std::memory_order_relaxed) && !m_Lock.exchange(true, std::memory_order_acquire);
    }

    void Spinlock::unlock() noexcept {
        m_Lock.store(false, std::memory_order_release);
    }
}

#endif // HELENA_CONCURRENCY_SPINLOCK_IPP
