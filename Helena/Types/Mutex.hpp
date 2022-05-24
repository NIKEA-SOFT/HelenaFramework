#ifndef HELENA_TYPES_MUTEX_HPP
#define HELENA_TYPES_MUTEX_HPP

#include <mutex>
#include <atomic>

namespace Helena::Types
{
    class Mutex
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
        Mutex() noexcept = default;
        ~Mutex() noexcept = default;
        Mutex(const Mutex&) = delete;
        Mutex(Mutex&&) noexcept = delete;
        Mutex& operator=(const Mutex&) = delete;
        Mutex& operator=(Mutex&&) noexcept = delete;

        void Lock() noexcept {
            while(m_Lock.test_and_set(std::memory_order_acquire)) {
                m_Lock.wait(true);
            }
        }

        void Unlock() noexcept {
            m_Lock.clear(std::memory_order_relaxed);
            m_Lock.notify_one();
        }

    private:
        std::atomic_flag m_Lock {};
    };
}
#endif // HELENA_TYPES_MUTEX_HPP