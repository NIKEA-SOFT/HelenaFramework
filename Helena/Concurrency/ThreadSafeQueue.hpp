#ifndef HELENA_CONCURRENCY_THREADSAFEQUEUE_HPP
#define HELENA_CONCURRENCY_THREADSAFEQUEUE_HPP

#include <queue>
#include <mutex>

namespace Helena::Concurrency
{
    template <typename T>
    class ThreadSafeQueue {

    public:
        ThreadSafeQueue() = default;
        ~ThreadSafeQueue() { Clear(); }
        ThreadSafeQueue(const ThreadSafeQueue&) = default;
        ThreadSafeQueue(ThreadSafeQueue&&) noexcept = delete;
        ThreadSafeQueue& operator=(const ThreadSafeQueue&) = default;
        ThreadSafeQueue& operator=(ThreadSafeQueue&&) noexcept = delete;

        template <typename... Args>
        void Push(Args&&... args) {
            std::lock_guard lock{ m_Mutex };
            m_Container.emplace(std::forward<Args>(args)...);
        }

        [[nodiscard]] bool Pop(T& value) noexcept
        {
            std::lock_guard lock{ m_Mutex };

            if(m_Container.empty()) {
                return false;
            }

            value = std::move(m_Container.front());
            m_Container.pop();

            return true;
        }

        [[nodiscard]] std::size_t GetSize() const noexcept {
            std::lock_guard lock{ m_Mutex };
            return m_Container.size();
        }

        void Clear() {
            std::lock_guard lock{ m_Mutex };
            while(!m_Container.empty()) {
                m_Container.pop();
            }
        }

    private:
        std::queue<T> m_Container;
        std::mutex m_Mutex;
    };
}


#endif // HELENA_CONCURRENCY_THREADSAFEQUEUE_HPP
