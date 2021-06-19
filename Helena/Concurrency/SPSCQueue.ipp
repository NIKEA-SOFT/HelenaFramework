#ifndef HELENA_CONCURRENCY_SPSCQUEUE_IPP
#define HELENA_CONCURRENCY_SPSCQUEUE_IPP

#include <Helena/Concurrency/SPSCQueue.hpp>

namespace Helena::Concurrency {

    template <typename T, std::uint32_t Size>
    SPSCQueue<T, Size>::SPSCQueue()
    : m_Elements {std::make_unique<Node[]>(elements_size)},
      m_Head {},
      m_Tail {},
      m_Shutdown {false} {}

    template <typename T, std::uint32_t Size>
    SPSCQueue<T, Size>::~SPSCQueue() {
        m_Shutdown.store(true);
        while(try_dequeue().has_value()) {};
    }

    template <typename T, std::uint32_t Size>
    template <typename... Args>
    void SPSCQueue<T, Size>::emplace(size_type index, Args&&... args) HF_NOEXCEPT {
        static_assert(sizeof...(Args) > 0, "Args... pack is empty");
        static_assert(std::is_constructible<T, Args...>::value, "T must be constructible with Args&&...");

        auto& node = m_Elements[index % elements_size];
        if constexpr(std::is_aggregate_v<T>) {
            new (&node.m_Data) T{std::forward<Args>(args)...};
        } else {
            new (&node.m_Data) T(std::forward<Args>(args)...);
        }
    }

    template <typename T, std::uint32_t Size>
    template <typename... Args>
    HF_NODISCARD bool SPSCQueue<T, Size>::enqueue(Args&&... args) HF_NOEXCEPT(std::is_nothrow_constructible_v<T, Args...>) {
        const auto head = m_Head.load(std::memory_order_relaxed);
        while(head - m_Tail.load(std::memory_order_acquire) >= elements_size) {
            if(m_Shutdown.load(std::memory_order_acquire)) {
                return false;
            }
        }

        emplace(head % elements_size, std::forward<Args>(args)...);
        m_Head.store(head + 1, std::memory_order_release);
        return true;
    }

    template <typename T, std::uint32_t Size>
    template <typename... Args>
    HF_NODISCARD bool SPSCQueue<T, Size>::try_enqueue(Args&&... args) HF_NOEXCEPT(std::is_nothrow_constructible_v<T, Args...>) {
        const auto head = m_Head.load(std::memory_order_relaxed);
        if(head - m_Tail.load(std::memory_order_acquire) >= elements_size) {
            return false;
        }

        emplace(head % elements_size, std::forward<Args>(args)...);
        m_Head.store(head + 1, std::memory_order_release);
        return true;
    }

    template <typename T, std::uint32_t Size>
    HF_NODISCARD auto SPSCQueue<T, Size>::dequeue() HF_NOEXCEPT(std::is_nothrow_destructible_v<T>) -> std::optional<T> {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        while(m_Head.load(std::memory_order_acquire) - tail >= elements_size) {
            if(m_Shutdown.load(std::memory_order_acquire)) {
                return std::nullopt;
            }
        }

        auto& node = m_Elements[tail % elements_size];
        auto object = std::move(*std::launder(reinterpret_cast<pointer>(&(node.m_Data))));
        m_Tail.store(tail + 1, std::memory_order_release);
        return object;
    }

    template <typename T, std::uint32_t Size>
    HF_NODISCARD auto SPSCQueue<T, Size>::try_dequeue() HF_NOEXCEPT(std::is_nothrow_destructible_v<T>) -> std::optional<T> {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        if(m_Head.load(std::memory_order_acquire) - tail == 0u) {
            return std::nullopt;
        }

        auto& node = m_Elements[tail % elements_size];
        auto optional = std::make_optional<T>(std::move(*std::launder(reinterpret_cast<pointer>(&(node.m_Data)))));
        m_Tail.store(tail + 1, std::memory_order_release);
        return optional;
    }

    template <typename T, std::uint32_t Size>
    HF_NODISCARD bool SPSCQueue<T, Size>::empty() const HF_NOEXCEPT {
        return !size();
    }

    template <typename T, std::uint32_t Size>
    HF_NODISCARD auto SPSCQueue<T, Size>::size() const HF_NOEXCEPT -> typename SPSCQueue<T, Size>::size_type {
        return m_Head.load(std::memory_order_acquire) - m_Tail.load(std::memory_order_acquire);
    }

    template <typename T, std::uint32_t Size>
    HF_NODISCARD auto SPSCQueue<T, Size>::capacity() const HF_NOEXCEPT -> typename SPSCQueue<T, Size>::size_type {
        return elements_size;
    }
}

#endif // HELENA_CONCURRENCY_SPSCQUEUE_IPP
