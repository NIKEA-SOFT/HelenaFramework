#ifndef HELENA_CONCURRENCY_SPSCQUEUE_IPP
#define HELENA_CONCURRENCY_SPSCQUEUE_IPP

#include <Helena/Concurrency/SPSCQueue.hpp>
#include <Helena/Concurrency/Internal.hpp>

namespace Helena::Concurrency {

    template <typename T>
    SPSCQueue<T>::SPSCQueue(const size_type size)
        : m_Elements {std::make_unique<data_type[]>(Internal::round_up_to_power_of_2(size))},
          m_Head {},
          m_Tail {},
          m_Capacity {size} {}

    template <typename T>
    SPSCQueue<T>::~SPSCQueue() {
        while(pop());
    }

    template <typename T>
    template <typename... Args>
    [[nodiscard]] bool SPSCQueue<T>::push([[maybe_unused]] Args&&... args)
    {
        const auto head = m_Head.load(std::memory_order_relaxed);
        if(head - m_Tail.load(std::memory_order_acquire) >= m_Capacity) {
            return false;
        }

        if constexpr (std::is_integral_v<value_type>) {
            m_Elements[head % m_Capacity] = value_type(std::forward<Args>(args)...);
        } else {
            if constexpr(std::is_aggregate_v<value_type>) {
                new (&m_Elements[head % m_Capacity]) value_type{std::forward<Args>(args)...};
            } else {
                new (&m_Elements[head % m_Capacity]) value_type(std::forward<Args>(args)...);
            }
        }

        m_Head.store(head + 1, std::memory_order_release);
        return true;
    }

    template <typename T>
    [[nodiscard]] auto SPSCQueue<T>::front() const noexcept -> decltype(auto)
    {
        const auto tail = m_Tail.load(std::memory_order_relaxed);

        if constexpr (std::is_integral_v<T>) {
            return m_Elements[tail % m_Capacity];
        } else {
            return *std::launder(reinterpret_cast<pointer>(&m_Elements[tail % m_Capacity]));
        }
    }

    template <typename T>
    [[nodiscard]] bool SPSCQueue<T>::pop()
    {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        if(m_Head.load(std::memory_order_acquire) == tail) {
            return false;
        }

        if constexpr (!std::is_integral_v<value_type> && Internal::is_detected_v<fn_dtor, value_type>) {
            std::launder(reinterpret_cast<pointer>(&m_Elements[tail % m_Capacity]))->~value_type();
        }

        m_Tail.store(tail + 1, std::memory_order_release);
        return true;
    }

    template <typename T>
    [[nodiscard]] bool SPSCQueue<T>::pop(reference value)
    {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        if(m_Head.load(std::memory_order_acquire) == tail) {
            return false;
        }

        if constexpr (std::is_integral_v<value_type>) {
            value = m_Elements[tail % m_Capacity];
        } else {
            value = std::move(*std::launder(reinterpret_cast<pointer>(&m_Elements[tail % m_Capacity])));
        }

        m_Tail.store(tail + 1, std::memory_order_release);
        return true;
    }

    template <typename T>
    template <typename Func>
    [[nodiscard]] bool SPSCQueue<T>::view_and_pop(Func callback)
    {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        if(m_Head.load(std::memory_order_acquire) == tail) {
            return false;
        }

        if constexpr (std::is_integral_v<value_type>) {
            callback(m_Elements[tail % m_Capacity]);
        } else {
            auto& instance = *std::launder(reinterpret_cast<pointer>(&m_Elements[tail % m_Capacity]));
            callback(instance);

            if constexpr (Internal::is_detected_v<fn_dtor, value_type>) {
                instance.~value_type();
            }
        }

        m_Tail.store(tail + 1, std::memory_order_release);
        return true;
    }

    template <typename T>
    [[nodiscard]] bool SPSCQueue<T>::empty() const noexcept {
        return !size();
    }

    template <typename T>
    [[nodiscard]] auto SPSCQueue<T>::size() const noexcept -> size_type {
        return m_Head.load(std::memory_order_acquire) - m_Tail.load(std::memory_order_acquire);
    }

    template <typename T>
    [[nodiscard]] auto SPSCQueue<T>::capacity() const noexcept -> size_type {
        return m_Capacity;
    }
}

#endif // HELENA_CONCURRENCY_SPSCQUEUE_IPP
