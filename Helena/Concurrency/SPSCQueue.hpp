#ifndef HELENA_CONCURRENCY_SPSCQUEUE_HPP
#define HELENA_CONCURRENCY_SPSCQUEUE_HPP

#include <cstdint>
#include <memory>
#include <type_traits>
#include <atomic>

namespace Helena::Concurrency {

    template <typename T>
    class SPSCQueue final
    {
        using data_type         = std::conditional_t<std::is_integral_v<T>, T, std::aligned_storage_t<sizeof(T), alignof(T)>>;

        template <typename Type, typename... Args>
        using fn_dtor           = decltype(std::declval<T>().~T());

    public:
        using value_type        = T;
        using pointer           = T*;
        using const_pointer     = const T*;
        using reference         = T&;
        using const_reference   = const T&;
        using size_type         = std::uint32_t;

    public:
        explicit SPSCQueue(const size_type size = 1024);
        ~SPSCQueue();
        SPSCQueue(const SPSCQueue&) = delete;
        SPSCQueue(SPSCQueue&&) noexcept = delete;
        SPSCQueue& operator=(const SPSCQueue&) = delete;
        SPSCQueue& operator=(SPSCQueue&&) noexcept = delete;

        template <typename... Args>
        [[nodiscard]] bool push([[maybe_unused]] Args&&... args);

        [[nodiscard]] auto front() const noexcept -> decltype(auto);

        [[nodiscard]] bool pop();

        [[nodiscard]] bool pop(reference value);

        template <typename Func>
        [[nodiscard]] bool view_and_pop(Func callback);

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] auto size() const noexcept -> size_type;

        [[nodiscard]] auto capacity() const noexcept -> size_type;

    private:
        std::unique_ptr<data_type[]> m_Elements;
        std::atomic<size_type> m_Head;
        std::atomic<size_type> m_Tail;
        const size_type m_Capacity;
    };
}

#include <Helena/Concurrency/SPSCQueue.ipp>

#endif // HELENA_CONCURRENCY_SPSCQUEUE_HPP
