#ifndef HELENA_TYPES_FUNCTION_HPP
#define HELENA_TYPES_FUNCTION_HPP

#include <Helena/Types/CompressedPair.hpp>
#include <functional>

namespace Helena::Types
{
    class Function
    {
    private:
        struct BindingFront {};
        struct BindingBack {};

        template <typename Fn, typename... Args>
        static constexpr auto RequiresBinding =
            std::is_constructible_v<std::decay_t<Fn>, Fn> &&
            std::is_move_constructible_v<std::decay_t<Fn>> &&
            (std::is_constructible_v<std::decay_t<Args>, Args> && ...) &&
            (std::is_move_constructible_v<std::decay_t<Args>> && ...);

        template <std::size_t... Indexes, typename Fn, typename Tuple, typename... Args>
        [[nodiscard]] static constexpr decltype(auto) Invoke(BindingFront, std::index_sequence<Indexes...>, Fn&& fn, Tuple&& captured, Args&&... args)
            noexcept(noexcept(std::invoke(std::forward<Fn>(fn), std::get<Indexes>(std::forward<Tuple>(captured))...,
                std::forward<Args>(args)...))) {
            return std::invoke(std::forward<Fn>(fn), std::get<Indexes>(std::forward<Tuple>(captured))...,
                std::forward<Args>(args)...);
        }

        template <std::size_t... Indexes, typename Fn, typename Tuple, typename... Args>
        [[nodiscard]] static constexpr decltype(auto) Invoke(BindingBack, std::index_sequence<Indexes...>, Fn&& fn, Tuple&& captured, Args&&... args)
            noexcept(noexcept(std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...,
                std::get<Indexes>(std::forward<Tuple>(captured))...))) {
            return std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...,
                std::get<Indexes>(std::forward<Tuple>(captured))...);
        }

        template <typename BindingType, typename Fn, typename... T>
        struct FunctionBinding {
            using Sequence = std::index_sequence_for<T...>;

            template <typename Functor, typename... Args>
            constexpr explicit FunctionBinding(Functor&& fn, Args&&... args) noexcept
                : m_Pair(std::piecewise_construct,
                    std::forward_as_tuple(std::forward<Functor>(fn)),
                    std::forward_as_tuple(std::forward<Args>(args)...)) {}

            template <typename... Args>
            [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) &
                noexcept(noexcept(Invoke(BindingType{}, Sequence{}, m_Pair.First(), m_Pair.Second(), std::forward<Args>(args)...))) {
                return Invoke(BindingType{}, Sequence{}, m_Pair.First(), m_Pair.Second(), std::forward<Args>(args)...);
            }

            template <typename... Args>
            [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const&
                noexcept(noexcept(Invoke(BindingType{}, Sequence{}, m_Pair.First(), m_Pair.Second(), std::forward<Args>(args)...))) {
                return Invoke(BindingType{}, Sequence{}, m_Pair.First(), m_Pair.Second(), std::forward<Args>(args)...);
            }

            template <typename... Args>
            [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) &&
                noexcept(noexcept(Invoke(BindingType{}, Sequence{}, std::move(m_Pair.First()), std::move(m_Pair.Second()), std::forward<Args>(args)...))) {
                return Invoke(BindingType{}, Sequence{}, std::move(m_Pair.First()), std::move(m_Pair.Second()), std::forward<Args>(args)...);
            }

            template <typename... Args>
            [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const&&
                noexcept(noexcept(Invoke(BindingType{}, Sequence{}, std::move(m_Pair.First()), std::move(m_Pair.Second()), std::forward<Args>(args)...))) {
                return Invoke(BindingType{}, Sequence{}, std::move(m_Pair.First()), std::move(m_Pair.Second()), std::forward<Args>(args)...);
            }

        private:
            Types::CompressedPair<Fn, std::tuple<T...>> m_Pair;
        };

    public:
        /**
        * @brief Captures arguments to be passed to the beginning of the called function.
        * @tparam Fn Type of invocable function
        * @tparam Args Type of argument to capture
        * @param fn Invocable function
        * @param args Pack of arguments to capture
        * @return Returns a callable object with an overloaded operator()
        */
        template <typename Fn, typename... Args>
        requires RequiresBinding<Fn, Args...>
        [[nodiscard]] static constexpr auto BindFront(Fn&& fn, Args&&... args) noexcept {
            return FunctionBinding<BindingFront, std::decay_t<Fn>, std::decay_t<Args>...>(std::forward<Fn>(fn), std::forward<Args>(args)...);
        }

        /**
        * @brief Captures arguments to pass to the end of the called function.
        * @tparam Fn Type of invocable function
        * @tparam Args Type of argument to capture
        * @param fn Invocable function
        * @param args Pack of arguments to capture
        * @return Returns a callable object with an overloaded operator()
        */
        template <typename Fn, typename... Args>
        [[nodiscard]] static constexpr auto BindBack(Fn&& fn, Args&&... args) noexcept {
            return FunctionBinding<BindingBack, std::decay_t<Fn>, std::decay_t<Args>...>(std::forward<Fn>(fn), std::forward<Args>(args)...);
        }
    };
}

#endif // HELENA_TYPES_FUNCTION_HPP