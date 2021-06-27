#ifndef HELENA_CONCURRENCY_SPSCQUEUE_IPP
#define HELENA_CONCURRENCY_SPSCQUEUE_IPP

#include <Helena/Concurrency/SPSCQueue.hpp>
#include <Helena/Concurrency/Internal.hpp>


namespace Helena::Concurrency {

    template <typename T>
    inline SPSCQueue<T>::SPSCQueue(const size_type size)
    : m_Elements {std::make_unique<data_type[]>(Internal::round_up_to_power_of_2(size))},
          m_Head {},
          m_Tail {},
          m_Capacity {size} {}

    template <typename T>
    inline SPSCQueue<T>::~SPSCQueue() {
        while(!empty()) {
            pop();
        }
    }

    template <typename T>
    template <typename... Args>
    [[nodiscard]] inline bool SPSCQueue<T>::push([[maybe_unused]] Args&&... args) {
        const auto head = m_Head.load(std::memory_order_relaxed);
        if(head - m_Tail.load(std::memory_order_acquire) >= m_Capacity) {
            return false;
        }

        if constexpr (std::is_integral_v<T>) {
            m_Elements[head % m_Capacity] = T(std::forward<Args>(args)...);
        } else {
            if constexpr(std::is_aggregate_v<T>) {
                new (&m_Elements[head % m_Capacity]) T{std::forward<Args>(args)...};
            } else {
                new (&m_Elements[head % m_Capacity]) T(std::forward<Args>(args)...);
            }
        }

        m_Head.store(head + 1, std::memory_order_release);
        return true;
    }

    template <typename T>
    [[nodiscard]] inline auto SPSCQueue<T>::front() const noexcept -> decltype(auto) {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        return *std::launder(reinterpret_cast<pointer>(&m_Elements[tail % m_Capacity]));
    }

    template <typename T>
    inline void SPSCQueue<T>::pop() noexcept {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        if(m_Head.load(std::memory_order_acquire) == tail) {
            return;
        }

        if constexpr (!std::is_integral_v<value_type>) {
            std::launder(reinterpret_cast<pointer>(&m_Elements[tail % m_Capacity]))->~value_type();
        }

        m_Tail.store(tail + 1, std::memory_order_release);
    }

    template <typename T>
    template <typename Func>
    inline bool SPSCQueue<T>::view_and_pop(Func&& callback) noexcept {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        if(m_Head.load(std::memory_order_acquire) == tail) {
            return false;
        }

        auto& instance = *std::launder(reinterpret_cast<pointer>(&m_Elements[tail % m_Capacity]));
        callback(instance);

        if constexpr (!std::is_integral_v<value_type>) {
            instance.~value_type();
        }

        m_Tail.store(tail + 1, std::memory_order_release);
        return true;
    }

    template <typename T>
    [[nodiscard]] inline bool SPSCQueue<T>::empty() const noexcept {
        return !size();
    }

    template <typename T>
    [[nodiscard]] inline auto SPSCQueue<T>::size() const noexcept -> size_type {
        return m_Head.load(std::memory_order_acquire) - m_Tail.load(std::memory_order_acquire);
    }

    template <typename T>
    [[nodiscard]] inline auto SPSCQueue<T>::capacity() const noexcept -> size_type {
        return m_Capacity;
    }
}

#endif // HELENA_CONCURRENCY_SPSCQUEUE_IPP
