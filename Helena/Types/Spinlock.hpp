#ifndef HELENA_TYPES_SPINLOCK_HPP
#define HELENA_TYPES_SPINLOCK_HPP

#include <Helena/Platform/Defines.hpp>

#include <mutex>
#include <atomic>

namespace Helena::Types
{
    class Spinlock
    {
        template <typename Mutex>
        friend class std::unique_lock;

        template <typename Mutex>
        friend class std::lock_guard;

        template <typename... Mutex>
        friend class std::scoped_lock;

        void lock() noexcept { Lock(); }
        void unlock() noexcept { Unlock(); }

    public:
        Spinlock() noexcept = default;
        ~Spinlock() noexcept = default;
        Spinlock(const Spinlock&) = delete;
        Spinlock(Spinlock&&) noexcept = delete;
        Spinlock& operator=(const Spinlock&) = delete;
        Spinlock& operator=(Spinlock&&) noexcept = delete;

        void Lock() noexcept 
        {
            while(true)
            {
                if(!m_Lock.exchange(true, std::memory_order_acquire)) {
                    return;
                }

                while(m_Lock.load(std::memory_order_relaxed)) {
                    HELENA_PROCESSOR_YIELD();
                }
            }
        }

        [[nodiscard]] bool TryLock() noexcept {
            return !m_Lock.load(std::memory_order_relaxed) && !m_Lock.exchange(true, std::memory_order_acquire);
        }

        void Unlock() noexcept {
            m_Lock.store(false, std::memory_order_release);
        }

    private:
        std::atomic<bool> m_Lock {};
    };
}
#endif // HELENA_TYPES_SPINLOCK_HPP
