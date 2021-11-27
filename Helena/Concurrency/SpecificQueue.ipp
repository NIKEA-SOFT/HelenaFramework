#ifndef HELENA_CONCURRENCY_SPECIFICQUEUE_IPP
#define HELENA_CONCURRENCY_SPECIFICQUEUE_IPP

#include <Helena/Concurrency/SpecificQueue.hpp>
#include <Helena/Debug/Assert.hpp>

namespace Helena::Concurrency {


    template <typename T, std::size_t Capacity>
    SpecificQueue<T, Capacity>::SpecificQueue() : m_Elements{}, m_Size{0u} {}

    template <typename T, std::size_t Capacity>
    SpecificQueue<T, Capacity>::~SpecificQueue()
    {
        if constexpr(!std::is_integral_v<value_type> && Internal::is_detected_v<fn_dtor, value_type>)
        {
            for(size_type i = 0u; i < m_Size; ++i) {
                auto& object = *std::launder(reinterpret_cast<pointer>(&m_Elements[i]));
                object.~value_type();
            }
        }

        m_Size = 0u;
    }

    template <typename T, std::size_t Capacity>
    SpecificQueue<T, Capacity>::SpecificQueue(const SpecificQueue& other) : m_Size{0u}
    {
        for(size_type i = 0u; i < other.size(); ++i)
        {
            if constexpr(std::is_integral_v<value_type>) {
                m_Elements[i] = other.m_Elements[i];
            } else {
                const auto& object = *std::launder(reinterpret_cast<const_pointer>(&other.m_Elements[i]));

                if constexpr(std::is_aggregate_v<value_type>) {
                    new (&m_Elements[i]) value_type{object};
                } else {
                    new (&m_Elements[i]) value_type(object);
                }
            }
        }

        m_Size = other.m_Size;
    }

    template <typename T, std::size_t Capacity>
    SpecificQueue<T, Capacity>::SpecificQueue(SpecificQueue&& other) noexcept : m_Size{0u}
    {
        for(size_type i = 0u; i < other.size(); ++i)
        {
            if constexpr(std::is_integral_v<value_type>) {
                m_Elements[i] = std::move(other.m_Elements[i]);
            } else {
                auto& rhs = *std::launder(reinterpret_cast<pointer>(&other.m_Elements[i]));

                if constexpr(std::is_aggregate_v<value_type>) {
                    new (&m_Elements[i]) value_type{std::move(rhs)};
                } else {
                    new (&m_Elements[i]) value_type(std::move(rhs));
                }

                if constexpr(Internal::is_detected_v<fn_dtor, value_type>) {
                    rhs.~value_type();
                }
            }
        }

        m_Size = other.m_Size;
        other.m_Size = 0u;
    }

    template <typename T, std::size_t Capacity>
    auto SpecificQueue<T, Capacity>::operator=(const SpecificQueue& other) -> SpecificQueue&
    {
        for(size_type i = 0u; i < other.size(); ++i)
        {
            if constexpr(std::is_integral_v<value_type>) {
                m_Elements[i] = other.m_Elements[i];
            } else {
                auto& lhs = *std::launder(reinterpret_cast<pointer>(&m_Elements[i]));
                const auto& rhs = *std::launder(reinterpret_cast<const_pointer>(&other.m_Elements[i]));

                lhs = rhs;
            }
        }

        m_Size = other.m_Size;

        return *this;
    }

    template <typename T, std::size_t Capacity>
    auto SpecificQueue<T, Capacity>::operator=(SpecificQueue&& other) noexcept -> SpecificQueue&
    {
        for(size_type i = 0u; i < other.size(); ++i)
        {
            if constexpr(std::is_integral_v<value_type>) {
                m_Elements[i] = other.m_Elements[i];
            } else {
                auto& lhs = *std::launder(reinterpret_cast<pointer>(&m_Elements[i]));
                auto& rhs = *std::launder(reinterpret_cast<const_pointer>(&other.m_Elements[i]));

                lhs = std::move(rhs);

                if constexpr(Internal::is_detected_v<fn_dtor, value_type>) {
                    rhs.~value_type();
                }
            }
        }

        m_Size = other.m_Size;
        other.m_Size = 0u;

        return *this;
    }

    template <typename T, std::size_t Capacity>
    template <typename... Args>
    void SpecificQueue<T, Capacity>::Push([[maybe_unused]] Args&&... args)
    {
        HELENA_ASSERT(m_Size < Capacity, "Buffer overflow");

        if constexpr(std::is_integral_v<value_type>) {
            m_Elements[m_Size++] = value_type(std::forward<Args>(args)...);
        } else {
            if constexpr(std::is_aggregate_v<value_type>) {
                new (&m_Elements[m_Size++]) value_type{std::forward<Args>(args)...};
            } else {
                new (&m_Elements[m_Size++]) value_type(std::forward<Args>(args)...);
            }
        }
    }

    template <typename T, std::size_t Capacity>
    template <typename Func>
    void SpecificQueue<T, Capacity>::Pop(Func callback)
    {
        for(size_type i = 0; i < m_Size; ++i)
        {
            if constexpr(std::is_integral_v<value_type>) {
                callback(m_Elements[i]);
            } else {
                auto& object = *std::launder(reinterpret_cast<pointer>(&m_Elements[i]));

                callback(object);

                if constexpr(Internal::is_detected_v<fn_dtor, value_type>) {
                    object.~value_type();
                }
            }
        }

        m_Size = 0u;
    }

    template <typename T, std::size_t Capacity>
    [[nodiscard]] bool SpecificQueue<T, Capacity>::IsFull() const noexcept {
        return m_Size == Capacity;
    }

    template <typename T, std::size_t Capacity>
    [[nodiscard]] bool SpecificQueue<T, Capacity>::IsEmpty() const noexcept {
        return !m_Size;
    }

    template <typename T, std::size_t Capacity>
    [[nodiscard]] auto SpecificQueue<T, Capacity>::GetSize() const noexcept -> size_type {
        return m_Size;
    }

    template <typename T, std::size_t Capacity>
    [[nodiscard]] auto SpecificQueue<T, Capacity>::GetRemainingSize() const noexcept -> size_type {
        return Capacity - m_Size;
    }

    template <typename T, std::size_t Capacity>
    [[nodiscard]] constexpr auto SpecificQueue<T, Capacity>::GetCapacity() noexcept -> size_type {
        return Capacity;
    }
}

#endif // HELENA_CONCURRENCY_SPECIFICQUEUE_IPP
