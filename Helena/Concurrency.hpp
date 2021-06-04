#ifndef HELENA_CONCURRENCY_HPP
#define HELENA_CONCURRENCY_HPP

#include <atomic>

namespace Helena::Concurrency
{
    class Spinlock final
    {
    public:
        void lock() noexcept
        {
            for (;;)
            {
                if (!m_Lock.exchange(true, std::memory_order_acquire)) {
                    return;
                }

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

#endif // HELENA_CONCURRENCY_HPP
