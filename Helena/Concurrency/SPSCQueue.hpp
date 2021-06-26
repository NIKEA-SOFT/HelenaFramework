#ifndef HELENA_CONCURRENCY_SPSCQUEUE_HPP
#define HELENA_CONCURRENCY_SPSCQUEUE_HPP

#include <Helena/Defines.hpp>
#include <Helena/Concurrency/Internal.hpp>

#include <atomic>
#include <memory>
#include <cstdint>


namespace Helena::Concurrency {

    template <typename T>
    class SPSCQueue final {
        using storage = std::conditional_t<std::is_integral_v<T>, T, std::aligned_storage_t<sizeof(T), alignof(T)>>;

    public:
        using value_type        = T;
        using pointer           = T*;
        using const_pointer     = const T*;
        using reference         = T&;
        using const_reference   = const T&;
        using size_type         = std::uint32_t;

    private:
        template <typename... Args>
        void emplace(size_type index, Args&&... args);
        auto extract(size_type index) -> decltype(auto);

    public:
        SPSCQueue(const size_type size = 1024);
        ~SPSCQueue();
        SPSCQueue(const SPSCQueue&) = delete;
        SPSCQueue& operator=(const SPSCQueue&) = delete;
        SPSCQueue(SPSCQueue&&) noexcept = delete;
        SPSCQueue& operator=(SPSCQueue&&) noexcept = delete;

        template <typename... Args>
        [[nodiscard]] bool push(Args&&... args);

        [[nodiscard]] auto front() const -> decltype(auto);

        void pop();

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] auto size() const noexcept -> size_type;

        [[nodiscard]] auto capacity() const noexcept -> size_type;

    private:
        std::unique_ptr<storage[]> m_Elements;
        std::atomic<size_type> m_Head;
        std::atomic<size_type> m_Tail;
        const size_type m_Capacity;
        std::atomic<bool> m_Shutdown;
    };
}

#include <Helena/Concurrency/SPSCQueue.ipp>

#endif // HELENA_CONCURRENCY_SPSCQUEUE_HPP
