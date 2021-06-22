#ifndef HELENA_CONCURRENCY_SPSCQUEUE_IPP
#define HELENA_CONCURRENCY_SPSCQUEUE_IPP

#include <Helena/Concurrency/SPSCQueue.hpp>
#include <Helena/Log.hpp>

namespace Helena::Concurrency {

    template <typename T, std::uint32_t Size>
    inline SPSCQueue<T, Size>::SPSCQueue()
    : m_Elements {std::make_unique<Node[]>(_capacity)},
      m_Head {},
      m_Tail {},
      m_Shutdown {false} {}

    template <typename T, std::uint32_t Size>
    inline SPSCQueue<T, Size>::~SPSCQueue() {
        m_Shutdown.store(true);
        while(try_dequeue().has_value()) {};
    }

    template <typename T, std::uint32_t Size>
    template <typename... Args>
    inline void SPSCQueue<T, Size>::emplace(size_type index, Args&&... args) {
        if constexpr(std::is_aggregate_v<T>) {
            new (&m_Elements[index].m_Data) T{std::forward<Args>(args)...};
        } else {
            new (&m_Elements[index].m_Data) T(std::forward<Args>(args)...);
        }
    }

    template <typename T, std::uint32_t Size>
    inline auto SPSCQueue<T, Size>::extract(size_type index) -> std::optional<T> {
        return std::make_optional<T>(std::move(*std::launder(reinterpret_cast<const T*>(&m_Elements[index].m_Data))));
    }

    template <typename T, std::uint32_t Size>
    template <typename... Args>
    [[nodiscard]] inline bool SPSCQueue<T, Size>::enqueue(Args&&... args) {
        const auto head = m_Head.load(std::memory_order_relaxed);
        while(head - m_Tail.load(std::memory_order_acquire) >= _capacity) {
            if(m_Shutdown.load(std::memory_order_acquire)) {
                return false;
            }
        }

        emplace(head % _capacity, std::forward<Args>(args)...);
        m_Head.store(head + 1, std::memory_order_release);
        return true;
    }

    template <typename T, std::uint32_t Size>
    template <typename... Args>
    [[nodiscard]] inline bool SPSCQueue<T, Size>::try_enqueue(Args&&... args) {
        const auto head = m_Head.load(std::memory_order_relaxed);
        if(head - m_Tail.load(std::memory_order_acquire) >= _capacity) {
            return false;
        }

        emplace(head % _capacity, std::forward<Args>(args)...);
        m_Head.store(head + 1, std::memory_order_release);
        return true;
    }

    template <typename T, std::uint32_t Size>
    [[nodiscard]] inline auto SPSCQueue<T, Size>::dequeue() -> std::optional<T> {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        while(m_Head.load(std::memory_order_acquire) - tail >= _capacity) {
            if(m_Shutdown.load(std::memory_order_acquire)) {
                return std::nullopt;
            }
        }

        auto optional = extract(tail % _capacity);
        m_Tail.store(tail + 1, std::memory_order_release);
        return optional;
    }

    template <typename T, std::uint32_t Size>
    [[nodiscard]] inline auto SPSCQueue<T, Size>::try_dequeue() -> std::optional<T> {
        const auto tail = m_Tail.load(std::memory_order_relaxed);
        if(m_Head.load(std::memory_order_acquire) - tail == 0u) {
            return std::nullopt;
        }

        auto optional = extract(tail % _capacity);
        m_Tail.store(tail + 1, std::memory_order_release);
        return optional;
    }

    template <typename T, std::uint32_t Size>
    [[nodiscard]] inline bool SPSCQueue<T, Size>::empty() const noexcept {
        return !size();
    }

    template <typename T, std::uint32_t Size>
    [[nodiscard]] inline auto SPSCQueue<T, Size>::size() const noexcept -> typename SPSCQueue<T, Size>::size_type {
        return m_Head.load(std::memory_order_acquire) - m_Tail.load(std::memory_order_acquire);
    }

    template <typename T, std::uint32_t Size>
    [[nodiscard]] inline auto SPSCQueue<T, Size>::capacity() const noexcept -> typename SPSCQueue<T, Size>::size_type {
        return _capacity;
    }
}

#endif // HELENA_CONCURRENCY_SPSCQUEUE_IPP
