#ifndef HELENA_CONCURRENCY_SPINLOCK_HPP
#define HELENA_CONCURRENCY_SPINLOCK_HPP

#include <atomic>

namespace Helena::Concurrency
{
    class Spinlock final
    {
    public:
        Spinlock() : m_Lock{} {}
        ~Spinlock() = default;
        Spinlock(const Spinlock&) = delete;
        Spinlock(Spinlock&&) noexcept = delete;
        Spinlock& operator=(const Spinlock&) = delete;
        Spinlock& operator=(Spinlock&&) noexcept = delete;

        void lock() noexcept
        {
            for (;;)
            {
                if (!m_Lock.exchange(true, std::memory_order_acquire)) {
                    return;
                }

                // todo: add sleep if long time spined
                while (m_Lock.load(std::memory_order_relaxed)) {}
            }
        }

        bool try_lock() noexcept {
            return !m_Lock.load(std::memory_order_relaxed) && !m_Lock.exchange(true, std::memory_order_acquire);
        }

        void unlock() noexcept {
            m_Lock.store(false, std::memory_order_release);
        }

    private:
        std::atomic<bool> m_Lock {};
    };
}

#endif // HELENA_CONCURRENCY_SPINLOCK_HPP
