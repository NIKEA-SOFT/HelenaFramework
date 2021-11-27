#ifndef HELENA_CONCURRENCY_SPECIFICQUEUE_HPP
#define HELENA_CONCURRENCY_SPECIFICQUEUE_HPP

#include <cstdint>
#include <memory>
#include <type_traits>

namespace Helena::Concurrency {

    template <typename T, std::size_t Capacity>
    class SpecificQueue final
    {
        static_assert(Capacity, "Capacity cannot be null");

        using data_type         = std::conditional_t<std::is_integral_v<T>, T, std::aligned_storage_t<sizeof(T), alignof(T)>>;

        template <typename Type, typename... Args>
        using fn_dtor           = decltype(std::declval<T>().~T());

    public:
        using value_type        = T;
        using pointer           = T*;
        using const_pointer     = const T*;
        using reference         = T&;
        using const_reference   = const T&;
        using size_type         = decltype(Capacity);

    public:
        SpecificQueue();
        ~SpecificQueue();
        SpecificQueue(const SpecificQueue&);
        SpecificQueue(SpecificQueue&&) noexcept;
        auto operator=(const SpecificQueue&) -> SpecificQueue&;
        auto operator=(SpecificQueue&&) noexcept -> SpecificQueue&;

        template <typename... Args>
        void Push([[maybe_unused]] Args&&... args);

        template <typename Func>
        void Pop(Func callback);

        [[nodiscard]] bool IsFull() const noexcept;
        [[nodiscard]] bool IsEmpty() const noexcept;

        [[nodiscard]] auto GetSize() const noexcept -> size_type;
        [[nodiscard]] auto GetRemainingSize() const noexcept -> size_type;
        [[nodiscard]] static constexpr auto GetCapacity() noexcept -> size_type;

    private:
        data_type m_Elements[Capacity];
        size_type m_Size;
    };
}

#include <Helena/Concurrency/SpecificQueue.ipp>

#endif // HELENA_CONCURRENCY_SPECIFICQUEUE_HPP
