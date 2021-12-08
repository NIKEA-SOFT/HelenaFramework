#ifndef HELENA_TYPES_MUTEX_HPP
#define HELENA_TYPES_MUTEX_HPP

#include <atomic>

namespace Helena::Types
{
    // TTAS implementation
    class Mutex
    {
    public:
        Mutex() = default;
        ~Mutex() = default;
        Mutex(const Mutex&) = delete;
        Mutex(Mutex&&) noexcept = delete;
        Mutex& operator=(const Mutex&) = delete;
        Mutex& operator=(Mutex&&) noexcept = delete;

        void Lock() noexcept {
            while(m_Lock.test_and_set(std::memory_order_acquire)) {
                m_Lock.wait(true);
            }
        }

        [[nodiscard]] bool TryLock() noexcept 
        {
            if(m_Lock.test_and_set(std::memory_order_acquire)) {
                m_Lock.wait(true);
                return true;
            }

            return false;
        }

        void Unlock() noexcept {
            m_Lock.clear(std::memory_order_release);
            m_Lock.notify_one();
        }

    private:
        std::atomic_flag m_Lock {};
    };
}
#endif // HELENA_TYPES_MUTEX_HPP