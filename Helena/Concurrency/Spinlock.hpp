#ifndef HELENA_CONCURRENCY_SPINLOCK_HPP
#define HELENA_CONCURRENCY_SPINLOCK_HPP

#include <Helena/Defines.hpp>

#include <atomic>

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
        std::atomic<bool> m_Lock {};
    };
}

#include <Helena/Concurrency/Spinlock.hpp>

#endif // HELENA_CONCURRENCY_SPINLOCK_HPP
