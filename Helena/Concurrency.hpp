#ifndef HELENA_CONCURRENCY_HPP
#define HELENA_CONCURRENCY_HPP

#include <atomic>
#include <queue>
#include <mutex>

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

    template <typename Type>
    class ThreadsafeQueue {
        using container_type    = std::queue<Type>;
        using value_type        = typename container_type::value_type;
        using reference         = typename container_type::reference;
        using const_reference   = typename container_type::const_reference;
        using size_type         = typename container_type::size_type;

    public:
        bool empty() const {
            return m_Objects.empty();
        }

        size_type size() const {
            return m_Objects.size();
        }

        reference front() {
            return m_Objects.front();
        }

        const_reference front() const {
            return m_Objects.front();
        }

        reference back() {
            return m_Objects.back();
        }

        const_reference back() const {
            return m_Objects.back();
        }

        void push(const Type& type) {
            std::lock_guard<std::mutex> lock{m_Mutex};
            m_Objects.push(type);
        }

        void push(Type&& type) {
            std::lock_guard<std::mutex> lock{m_Mutex};
            m_Objects.push(std::move(type));
        }

        template <typename... Args>
        void emplace(Args&&... args) {
            std::lock_guard<std::mutex> lock{m_Mutex};
            m_Objects.emplace(std::forward<Args>(args)...);
        }

        void pop() {
            std::lock_guard<std::mutex> lock{m_Mutex};
            m_Objects.pop();
        }



    private:
        container_type m_Objects;
        std::mutex m_Mutex;
    };
}

#endif // HELENA_CONCURRENCY_HPP
