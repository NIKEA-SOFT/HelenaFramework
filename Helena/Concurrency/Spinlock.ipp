#ifndef HELENA_CONCURRENCY_SPINLOCK_IPP
#define HELENA_CONCURRENCY_SPINLOCK_IPP

#include <Helena/Concurrency/Spinlock.hpp>

namespace Helena::Concurrency {

    Spinlock::Spinlock() : m_CounterLow{}, m_CounterHigh{} {}

    void Spinlock::lock() noexcept {
        const auto counter = m_CounterLow.fetch_add(1u);
        while(counter != m_CounterHigh.load()) {}
    }

    [[nodiscard]] bool Spinlock::try_lock() noexcept {
        auto counter = m_CounterHigh.load();
        return m_CounterLow.compare_exchange_strong(counter, counter + 1u);
    }

    void Spinlock::unlock() noexcept {
        m_CounterHigh.fetch_add(1u);
    }

}

#endif // HELENA_CONCURRENCY_SPINLOCK_IPP
