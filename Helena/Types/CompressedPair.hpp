#ifndef HELENA_TYPES_COMPRESSEDPAIR_HPP
#define HELENA_TYPES_COMPRESSEDPAIR_HPP

#include <tuple>
#include <utility>
#include <type_traits>
#include <concepts>

namespace Helena::Types
{
    namespace Internal {

        // Empty Base Optimization storage for compressed pair
        template <typename T, std::size_t, bool = std::conjunction_v<std::is_empty<T>, std::negation<std::is_final<T>>>>
        struct Storage;

        template <typename T, std::size_t Tag>
        struct Storage<T, Tag, false> {
            constexpr Storage()
                noexcept(std::is_nothrow_default_constructible_v<T>)
                requires(std::is_default_constructible_v<T>)
                : m_Value{} {};

            template<typename Value>
            requires(std::is_constructible_v<T, Value>)
            constexpr Storage(Value&& value) noexcept(std::is_nothrow_constructible_v<T, Value>)
                : m_Value{std::forward<Value>(value)} {}

            template <typename... Args, std::size_t... Index>
            requires(std::is_constructible_v<T, Args...>)
            constexpr Storage(std::tuple<Args...> tuple, std::index_sequence<Index...>)
                noexcept(std::is_nothrow_constructible_v<T, Args...>)
                : m_Value{std::forward<Args>(std::get<Index>(tuple))...} {}

            constexpr T& Get() noexcept {
                return m_Value;
            }

            constexpr const T& Get() const noexcept {
                return m_Value;
            }

            T m_Value;
        };

        template <typename Base, std::size_t Tag>
        struct Storage<Base, Tag, true> : private Base {
            constexpr Storage()
                noexcept(std::is_nothrow_default_constructible_v<Base>)
                requires(std::is_default_constructible_v<Base>)
                : Base{} {};

            template<typename Value>
            requires(std::is_constructible_v<Base, Value>)
            constexpr Storage(Value&& value) noexcept(std::is_nothrow_constructible_v<Base, Value>)
                : Base{std::forward<Value>(value)} {}

            template <typename... Args, std::size_t... Index>
            requires(std::is_constructible_v<Base, Args...>)
            constexpr Storage(std::tuple<Args...> tuple, std::index_sequence<Index...>)
                noexcept(std::is_nothrow_constructible_v<Base, Args...>)
                : Base{std::forward<Args>(std::get<Index>(tuple))...} {}

            constexpr Base& Get() noexcept {
                return *this;
            }

            constexpr const Base& Get() const noexcept {
                return *this;
            }
        };
    }

    template <typename T1, typename T2>
    class CompressedPair : private Internal::Storage<T1, 0>
                         , private Internal::Storage<T2, 1>
    {
        using base_first = Internal::Storage<T1, 0>;
        using base_second = Internal::Storage<T2, 1>;

    public:
        using first_type = T1;
        using second_type = T2;

        static constexpr std::size_t IndexFirst = 0;
        static constexpr std::size_t IndexSecond = 1;

    public:
        constexpr CompressedPair()
            noexcept(std::is_nothrow_default_constructible_v<base_first> && std::is_nothrow_default_constructible_v<base_second>)
            requires(std::is_default_constructible_v<base_first> && std::is_default_constructible_v<base_second>)
            : base_first{}, base_second{} {}

        constexpr CompressedPair(const CompressedPair&)
            noexcept(std::is_nothrow_copy_constructible_v<base_first> && std::is_nothrow_copy_constructible_v<base_second>) = default;

        constexpr CompressedPair(CompressedPair&&)
            noexcept(std::is_nothrow_move_constructible_v<base_first> && std::is_nothrow_move_constructible_v<base_second>) = default;

        template <typename First, typename Second>
        requires (std::is_constructible_v<base_first, const First&> && std::is_constructible_v<base_second, const Second&>)
        constexpr explicit(!std::conjunction_v<std::is_convertible<const First&, T1>, std::is_convertible<const Second&, T2>>)
        CompressedPair(const CompressedPair<First, Second>& other)
            noexcept(std::is_nothrow_constructible_v<base_first, First> && std::is_nothrow_constructible_v<base_second, Second>)
            : base_first{other.First()}
            , base_second{other.Second()} {}

        template <typename First, typename Second>
        requires (std::is_constructible_v<base_first, First> && std::is_constructible_v<base_second, Second>)
        constexpr explicit(!std::conjunction_v<std::is_convertible<First, T1>, std::is_convertible<Second, T2>>)
        CompressedPair(CompressedPair<First, Second>&& other)
            noexcept(std::is_nothrow_constructible_v<base_first, First> && std::is_nothrow_constructible_v<base_second, Second>)
            : base_first{std::forward<First>(other.First())}
            , base_second{std::forward<Second>(other.Second())} {}

        template<typename First, typename Second>
        requires(std::is_constructible_v<base_first, const First&> && std::is_constructible_v<base_second, const Second&>)
        constexpr explicit(!std::conjunction_v<std::is_convertible<First, T1>, std::is_convertible<Second, T2>>)
        CompressedPair(const First& first, const Second& second)
            noexcept(std::is_nothrow_constructible_v<base_first, First> && std::is_nothrow_constructible_v<base_second, Second>)
            : base_first{first}
            , base_second{second} {}

        template<typename First = T1, typename Second = T2>
        requires(std::conjunction_v<std::is_constructible<base_first, First>, std::is_constructible<base_second, Second>>)
        constexpr explicit(!std::conjunction_v<std::is_convertible<First, T1>, std::is_convertible<Second, T2>>)
        CompressedPair(First&& first, Second&& second)
            noexcept(std::is_nothrow_constructible_v<base_first, First> && std::is_nothrow_constructible_v<base_second, Second>)
            : base_first{std::forward<First>(first)}
            , base_second{std::forward<Second>(second)} {}

        template <typename... ArgsT1, typename... ArgsT2>
        constexpr CompressedPair(std::piecewise_construct_t, std::tuple<ArgsT1...> first, std::tuple<ArgsT2...> second)
            noexcept(std::is_nothrow_constructible_v<T1, ArgsT1...> && std::is_nothrow_constructible_v<T2, ArgsT2...>)
            : base_first{std::move(first), std::index_sequence_for<ArgsT1...>{}}
            , base_second{std::move(second), std::index_sequence_for<ArgsT2...>{}} {}

        constexpr CompressedPair& operator=(const CompressedPair& other)
            noexcept(std::is_nothrow_copy_assignable_v<T1> && std::is_nothrow_copy_assignable_v<T2>) = default;

        constexpr CompressedPair& operator=(CompressedPair&& other)
            noexcept(std::is_nothrow_move_assignable_v<T1> && std::is_nothrow_move_assignable_v<T2>) = default;

        template <typename First, typename Second>
        requires (!std::same_as<CompressedPair, CompressedPair<First, Second>>
                && std::is_assignable_v<T1&, const First&> && std::is_assignable_v<T2&, const Second&>)
        constexpr CompressedPair& operator=(const CompressedPair<First, Second>& other)
            noexcept(std::is_nothrow_assignable_v<T1&, const First&> && std::is_nothrow_assignable_v<T2&, const Second&>) {
            this->First() = other.First();
            this->Second() = other.Second();
        }

        template <typename First, typename Second>
        requires (!std::same_as<CompressedPair, CompressedPair<First, Second>>
                && std::is_assignable_v<T1&, First> && std::is_assignable_v<T2&, Second>)
        constexpr CompressedPair& operator=(CompressedPair<First, Second>&& other)
            noexcept(std::is_nothrow_assignable_v<T1&, First> && std::is_nothrow_assignable_v<T2&, Second>) {
            this->First() = std::move(other.First());
            this->Second() = std::move(other.Second());
        }

        [[nodiscard]] constexpr decltype(auto) First() noexcept {
            return static_cast<base_first*>(this)->Get();
        }

        [[nodiscard]] constexpr decltype(auto) First() const noexcept {
            return static_cast<const base_first*>(this)->Get();
        }

        [[nodiscard]] constexpr decltype(auto) Second() noexcept {
            return static_cast<base_second*>(this)->Get();
        }

        [[nodiscard]] constexpr decltype(auto) Second() const noexcept {
            return static_cast<const base_second*>(this)->Get();
        }

        template <typename T>
        requires (!std::same_as<T1, T2> && (std::same_as<T, T1> || std::same_as<T, T2>))
        [[nodiscard]] constexpr decltype(auto) Get() noexcept {
            if constexpr(std::is_same_v<T, T1>) {
                return First();
            } else if constexpr(std::is_same_v<T, T2>) {
                return Second();
            }
        }

        template <typename T>
        requires (!std::same_as<T1, T2> && (std::same_as<T, T1> || std::same_as<T, T2>))
        [[nodiscard]] constexpr decltype(auto) Get() const noexcept {
            if constexpr(std::is_same_v<T, T1>) {
                return First();
            } else if constexpr(std::is_same_v<T, T2>) {
                return Second();
            }
        }

        template <std::size_t Index>
        requires (std::cmp_less_equal(Index, IndexSecond))
        [[nodiscard]] constexpr decltype(auto) Get(std::in_place_index_t<Index>) noexcept {
            if constexpr(Index == IndexFirst) {
                return First();
            } else if constexpr(Index == IndexSecond) {
                return Second();
            }
        }

        template <std::size_t Index>
        requires (std::cmp_less_equal(Index, IndexSecond))
        [[nodiscard]] constexpr decltype(auto) Get(std::in_place_index_t<Index>) const noexcept {
            if constexpr(Index == IndexFirst) {
                return First();
            } else if constexpr(Index == IndexSecond) {
                return Second();
            }
        }
    };

    template<typename First, typename Second>
    CompressedPair(First, Second) -> CompressedPair<First, Second>;

    template<typename First, typename Second>
    CompressedPair(CompressedPair<First, Second>) -> CompressedPair<std::decay_t<First>, std::decay_t<Second>>;
}
#endif // HELENA_TYPES_COMPRESSEDPAIR_HPP