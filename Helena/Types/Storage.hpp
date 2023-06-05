#ifndef HELENA_TYPES_STORAGE_HPP
#define HELENA_TYPES_STORAGE_HPP

#include <memory>
#include <type_traits>
#include <utility>

#include <Helena/Platform/Assert.hpp>

namespace Helena::Types
{
    template <typename>
    struct Storage;

    template <>
    struct Storage<void>
    {
        template <typename T>
        struct NonTrivialCopy : T {
            using T::T;

            NonTrivialCopy() = default;

            constexpr NonTrivialCopy(const NonTrivialCopy& other) noexcept(
                noexcept(T::ConstructFrom(static_cast<const T&>(other)))) {
                T::ConstructFrom(static_cast<const T&>(other));
            }

            NonTrivialCopy(NonTrivialCopy&&) = default;
            NonTrivialCopy& operator=(const NonTrivialCopy&) = default;
            NonTrivialCopy& operator=(NonTrivialCopy&&) = default;
        };

        template <typename T, typename... Args>
        using ControlCopy =
            std::conditional_t<std::conjunction_v<
            std::is_copy_constructible<Args>...,
            std::negation<std::conjunction<
            std::is_trivially_copy_constructible<Args>...>>>,
            NonTrivialCopy<T>, T>;

        template <typename T, typename... Args>
        struct NonTrivialMove : ControlCopy<T, Args...> {
            using Base = ControlCopy<T, Args...>;
            using Base::Base;

            NonTrivialMove() = default;
            NonTrivialMove(const NonTrivialMove&) = default;

            constexpr NonTrivialMove(NonTrivialMove&& other) noexcept(
                noexcept(Base::ConstructFrom(static_cast<Base&&>(other)))) {
                Base::ConstructFrom(static_cast<Base&&>(other));
            }

            NonTrivialMove& operator=(const NonTrivialMove&) = default;
            NonTrivialMove& operator=(NonTrivialMove&&) = default;
        };

        template <typename T, typename... Args>
        using ControlMove =
            std::conditional_t<std::conjunction_v<
            std::is_move_constructible<Args>...,
            std::negation<std::conjunction<
            std::is_trivially_move_constructible<Args>...>>>,
            NonTrivialMove<T>, ControlCopy<T, Args...>>;

        template <typename T, typename... Args>
        struct NonTrivialCopyAssign : ControlMove<T, Args...> {
            using Base = ControlMove<T, Args...>;
            using Base::Base;

            NonTrivialCopyAssign() = default;
            NonTrivialCopyAssign(const NonTrivialCopyAssign&) = default;
            NonTrivialCopyAssign(NonTrivialCopyAssign&&) = default;

            constexpr NonTrivialCopyAssign& operator=(const NonTrivialCopyAssign& other) noexcept(
                noexcept(Base::AssignFrom(static_cast<const Base&>(other)))) {
                Base::AssignFrom(static_cast<const Base&>(other));
                return *this;
            }

            NonTrivialCopyAssign& operator=(NonTrivialCopyAssign&&) = default;
        };

        template <typename T, typename... Args>
        struct DeletedCopyAssign : ControlMove<T, Args...> {
            using Base = ControlMove<T, Args...>;
            using Base::Base;

            DeletedCopyAssign() = default;
            DeletedCopyAssign(const DeletedCopyAssign&) = default;
            DeletedCopyAssign(DeletedCopyAssign&&) = default;
            DeletedCopyAssign& operator=(const DeletedCopyAssign&) = delete;
            DeletedCopyAssign& operator=(DeletedCopyAssign&&) = default;
        };

        template <typename T, typename... Args>
        using ControlCopyAssign =
            std::conditional_t<std::conjunction_v<
            std::is_trivially_destructible<Args>...,
            std::is_trivially_copy_constructible<Args>...,
            std::is_trivially_copy_assignable<Args>...>,
            ControlMove<T, Args...>,
            std::conditional_t<std::conjunction_v<
            std::is_copy_constructible<Args>...,
            std::is_copy_assignable<Args>...>,
            NonTrivialCopyAssign<T, Args...>,
            DeletedCopyAssign<T, Args...>>>;

        template <typename T, typename... Args>
        struct NonTrivialMoveAssign : ControlCopyAssign<T, Args...> {
            using Base = ControlCopyAssign<T, Args...>;
            using Base::Base;

            NonTrivialMoveAssign() = default;
            NonTrivialMoveAssign(const NonTrivialMoveAssign&) = default;
            NonTrivialMoveAssign(NonTrivialMoveAssign&&) = default;
            NonTrivialMoveAssign& operator=(const NonTrivialMoveAssign&) = default;

            constexpr NonTrivialMoveAssign& operator=(NonTrivialMoveAssign&& other) noexcept(
                noexcept(Base::AssignFrom(static_cast<Base&&>(other)))) {
                Base::AssignFrom(static_cast<Base&&>(other));
                return *this;
            }
        };

        template <typename T, typename... Args>
        struct DeletedMoveAssign : ControlCopyAssign<T, Args...> {
            using Base = ControlCopyAssign<T, Args...>;
            using Base::Base;

            DeletedMoveAssign() = default;
            DeletedMoveAssign(const DeletedMoveAssign&) = default;
            DeletedMoveAssign(DeletedMoveAssign&&) = default;
            DeletedMoveAssign& operator=(const DeletedMoveAssign&) = default;
            DeletedMoveAssign& operator=(DeletedMoveAssign&&) = delete;
        };

        template <typename T, typename... Args>
        using ControlMoveAssign =
            std::conditional_t<std::conjunction_v<
            std::is_trivially_destructible<Args>...,
            std::is_trivially_move_constructible<Args>...,
            std::is_trivially_move_assignable<Args>...>,
            ControlCopyAssign<T, Args...>,
            std::conditional_t<std::conjunction_v<
            std::is_move_constructible<Args>...,
            std::is_move_assignable<Args>...>,
            NonTrivialMoveAssign<T, Args...>,
            DeletedMoveAssign<T, Args...>>>;

        template <typename T, typename... Args>
        using ControlSwitcher = ControlMoveAssign<T, Args...>;

        struct Dummy { constexpr Dummy() noexcept {}};
        static_assert(!std::is_trivially_default_constructible_v<Dummy>);

        template <typename T, bool = std::is_trivially_destructible_v<T>>
        struct Destructible {
            constexpr Destructible() noexcept : m_Dummy{}, m_HasValue{false} {}

            template <typename... Args>
            constexpr explicit Destructible(std::in_place_t, Args&&... args)
                : m_Value(std::forward<Args>(args)...), m_HasValue{true} {}

            constexpr void Reset() noexcept {
                m_HasValue = false;
            }

            union {
                Dummy m_Dummy;
                std::remove_const_t<T> m_Value;
            };
            bool m_HasValue;
        };

        template <typename T>
        struct Destructible<T, false> {
            constexpr Destructible() noexcept : m_Dummy{}, m_HasValue{false} {}

            template <typename... Args>
            constexpr explicit Destructible(std::in_place_t, Args&&... args)
                : m_Value(std::forward<Args>(args)...), m_HasValue{true} {}

            constexpr ~Destructible() noexcept {
                if(m_HasValue) {
                    std::destroy_at(std::addressof(m_Value));
                }
            }

            Destructible(const Destructible&) = default;
            Destructible(Destructible&&) = default;
            Destructible& operator=(const Destructible&) = default;
            Destructible& operator=(Destructible&&) = default;

            constexpr void Reset() noexcept {
                if(m_HasValue) {
                    std::destroy_at(std::addressof(m_Value));
                    m_HasValue = false;
                }
            }

            union {
                Dummy m_Dummy;
                std::remove_const_t<T> m_Value;
            };
            bool m_HasValue;
        };

        template <typename T>
        struct Constructible : Destructible<T> {
            using Destructible<T>::Destructible;

            template <typename... Args>
            constexpr T& Construct(Args&&... args) {
                HELENA_ASSERT(!this->m_HasValue);
                this->m_HasValue = true;
                return *std::construct_at(std::addressof(this->m_Value), std::forward<Args>(args)...);
            }

            template <typename Self>
            constexpr void ConstructFrom(Self&& other)
                noexcept(std::is_nothrow_constructible_v<T, decltype(*std::forward<Self>(other))>) {
                if(other.m_HasValue) {
                    Construct(*std::forward<Self>(other));
                }
            }

            template <typename Self>
            constexpr void AssignFrom(Self&& other) noexcept(
                std::is_nothrow_constructible_v<T, decltype(*std::forward<Self>(other))> &&
                std::is_nothrow_assignable_v<T&, decltype(*std::forward<Self>(other))>) {
                if (other.m_HasValue) {
                    Assign(*std::forward<Self>(other));
                } else {
                    this->Reset();
                }
            }

            template <typename R>
            constexpr void Assign(R&& value) {
                if(this->m_HasValue) {
                    this->m_Value = std::forward<R>(value);
                } else {
                    Construct(std::forward<R>(value));
                }
            }

            [[nodiscard]] constexpr T& operator*() & noexcept {
                HELENA_ASSERT(this->m_HasValue);
                return this->m_Value;
            }

            [[nodiscard]] constexpr const T& operator*() const& noexcept {
                HELENA_ASSERT(this->m_HasValue);
                return this->m_Value;
            }

            [[nodiscard]] constexpr T&& operator*() && noexcept {
                HELENA_ASSERT(this->m_HasValue);
                return std::move(this->m_Value);
            }

            [[nodiscard]] constexpr const T&& operator*() const&& noexcept {
                HELENA_ASSERT(this->m_HasValue);
                return std::move(this->m_Value);
            }
        };

        template <typename T>
        using Switcher = ControlSwitcher<Constructible<T>, T>;
    };

    // Storage with dummy constructor similar to that used in std::optional
    template <typename T>
    struct Storage : Storage<void>::Switcher<T> {
        using Base = typename Storage<void>::Switcher<T>;
        using Base::Base;
    };
}

#endif // HELENA_TYPES_STORAGE_HPP