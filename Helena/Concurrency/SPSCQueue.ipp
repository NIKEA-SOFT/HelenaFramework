#ifndef HELENA_CONCURRENCY_SPSCQUEUE_IPP
#define HELENA_CONCURRENCY_SPSCQUEUE_IPP

#include <Helena/Concurrency/SPSCQueue.hpp>
#include <Helena/Log.hpp>

namespace Helena::Concurrency {

    template <typename T>
    inline SPSCQueue<T>::SPSCQueue(const size_type size)
    : m_Elements {std::make_unique<storage[]>(size)},
      m_Head {},
      m_Tail {},
      m_Capacity {size},
      m_Shutdown {false} {}

    template <typename T>
    inline SPSCQueue<T>::~SPSCQueue() {
        m_Shutdown.store(true);

        T value {};
        while(!empty()) {
            (void)try_dequeue(value);
        };
    }

    template <typename T>
    template <typename... Args>
    inline void SPSCQueue<T>::emplace(size_type index, Args&&... args) {
        if constexpr (std::is_integral_v<T>) {
            m_Elements[index] = T(std::forward<Args>(args)...);
        } else {
            if constexpr(std::is_aggregate_v<T>) {
                new (&m_Elements[index]) T{std::forward<Args>(args)...};
            } else {
                new (&m_Elements[index]) T(std::forward<Args>(args)...);
            }
        }
    }

    template <typename T>
    inline auto SPSCQueue<T>::extract(size_type index) -> value_type&& {
        if constexpr (std::is_integral_v<T>) {
            return std::move(m_Elements[index]);
        } else {
            return std::move(*std::launder(reinterpret_cast<T*>(&m_Elements[index])));
        }
    }

    template <typename T>
    template <typename... Args>
    [[nodiscard]] inline bool SPSCQueue<T>::enqueue(Args&&... args) {
        const auto head = m_Head.load(std::memory_order_relaxed);
        while(head - m_Tail.load(std::memory_order_acquire) >= m_Capacity) {
            if(m_Shutdown.load(std::memory_order_acquire)) {
                return false;
            }
        }

        emplace(head % m_Capacity, std::forward<Args>(args)...);
        m_Head.store(head + 1, std::memory_order_release);
        return true;
    }

    template <typename T>
    template <typename... Args>
    [[nodiscard]] inline bool SPSCQueue<T>::try_enqueue(Args&&... args) {
        const auto head = m_Head.load(std::memory_order_relaxed);
        if(head - m_Tail.load(std::memory_order_acquire) >= m_Capacity) {
            return false;
        }

        emplace(head % m_Capacity, std::forward<Args>(args)...);
        m_Head.store(head + 1, std::memory_order_release);
        return true;
    }

    template <typename T>
    [[nodiscard]] inline auto SPSCQueue<T>::dequeue(reference value) -> bool {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        while(m_Head.load(std::memory_order_acquire) == tail) {
            if(m_Shutdown.load(std::memory_order_acquire)) {
                return false;
            }
        }

        value = std::move(extract(tail % m_Capacity));
        m_Tail.store(tail + 1, std::memory_order_release);
        return true;
    }

    template <typename T>
    [[nodiscard]] inline auto SPSCQueue<T>::try_dequeue(reference value) -> bool {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        if(m_Head.load(std::memory_order_acquire) == tail) {
            return false;
        }

        value = std::move(extract(tail % m_Capacity));
        m_Tail.store(tail + 1, std::memory_order_release);
        return true;
    }

    template <typename T>
    [[nodiscard]] inline bool SPSCQueue<T>::empty() const noexcept {
        return !size();
    }

    template <typename T>
    [[nodiscard]] inline auto SPSCQueue<T>::size() const noexcept -> size_type {
        return m_Head - m_Tail;
    }

    template <typename T>
    [[nodiscard]] inline auto SPSCQueue<T>::capacity() const noexcept -> size_type {
        return m_Capacity;
    }
}

#endif // HELENA_CONCURRENCY_SPSCQUEUE_IPP
