#ifndef HELENA_CONCURRENCY_SPSCQUEUE_HPP
#define HELENA_CONCURRENCY_SPSCQUEUE_HPP

#include <Helena/Defines.hpp>
#include <Helena/Concurrency/Internal.hpp>

#include <atomic>
#include <memory>
#include <optional>
#include <cstdint>
#include <algorithm>

namespace Helena::Concurrency {

    template <typename T, std::uint32_t Size>
    class SPSCQueue final {
        using storage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

    public:
        using value_type        = T;
        using pointer           = T*;
        using const_pointer     = const T*;
        using reference         = T&;
        using const_reference   = const T&;
        using size_type         = std::uint32_t;

    private:
        constexpr static size_type _capacity = Internal::round_up_to_power_of_2(Size);
//        constexpr static size_type shuffle_bits = []() {
//            constexpr auto elementsPerCacheLine = Internal::cache_line / sizeof(T);
//            constexpr auto bitsIndex = Internal::log2(elementsPerCacheLine);
//            constexpr auto minSize = 1u << (bitsIndex * 2);
//            return elements_size < minSize ? 0u : bitsIndex;
//        }();

//        constexpr static size_type GetMixedIndex(const size_type index) {
//            if constexpr (shuffle_bits) {
//                const size_type mix = (index ^ (index >> shuffle_bits)) & ((1u << shuffle_bits) - 1);
//                return index ^ mix ^ (mix << shuffle_bits);
//            } else {
//                return index;
//            }
//        }

        struct alignas(Internal::cache_line) Node {
            storage m_Data {};
        };

    private:
        template <typename... Args>
        void emplace(size_type index, Args&&... args);
        auto extract(size_type index) -> std::optional<T>;

    public:
        SPSCQueue();
        ~SPSCQueue();
        SPSCQueue(const SPSCQueue&) = delete;
        SPSCQueue& operator=(const SPSCQueue&) = delete;
        SPSCQueue(SPSCQueue&&) noexcept = delete;
        SPSCQueue& operator=(SPSCQueue&&) noexcept = delete;

        template <typename... Args>
        [[nodiscard]] bool enqueue(Args&&... args);

        template <typename... Args>
        [[nodiscard]] bool try_enqueue(Args&&... args);

        [[nodiscard]] auto dequeue() -> std::optional<T>;

        [[nodiscard]] auto try_dequeue() -> std::optional<T>;

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] auto size() const noexcept -> size_type;

        [[nodiscard]] auto capacity() const noexcept -> size_type;

    private:
        std::unique_ptr<Node[]> m_Elements;
        std::atomic<size_type> m_Head;
        std::atomic<size_type> m_Tail;
        std::atomic<bool> m_Shutdown;
    };
}

#include <Helena/Concurrency/SPSCQueue.ipp>

#endif // HELENA_CONCURRENCY_SPSCQUEUE_HPP
