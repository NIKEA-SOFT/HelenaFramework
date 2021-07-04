#ifndef HELENA_CONCURRENCY_SPINLOCK_HPP
#define HELENA_CONCURRENCY_SPINLOCK_HPP

#include <Helena/Defines.hpp>
#include <Helena/Concurrency/Internal.hpp>

#include <cstdint>
#include <atomic>
#include <vector>
#include <thread>

namespace Helena::Concurrency
{
    class Spinlock final
    {        
    public:
        Spinlock();
        ~Spinlock() = default;
        Spinlock(const Spinlock&) = delete;
        Spinlock(Spinlock&&) noexcept = delete;
        Spinlock& operator=(const Spinlock&) = delete;
        Spinlock& operator=(Spinlock&&) noexcept = delete;

        void lock() noexcept;
        [[nodiscard]] bool try_lock() noexcept;
        void unlock() noexcept;

    private:
        std::atomic<std::uint32_t> m_CounterLow;
        char padding[Internal::cache_line - sizeof(m_CounterLow)];
        std::atomic<std::uint32_t> m_CounterHigh;
    };
}

#include <Helena/Concurrency/Spinlock.ipp>

#endif // HELENA_CONCURRENCY_SPINLOCK_HPP
