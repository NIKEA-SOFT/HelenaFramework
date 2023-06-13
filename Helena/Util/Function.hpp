#ifndef HELENA_UTIL_FUNCTION_HPP
#define HELENA_UTIL_FUNCTION_HPP

#include <functional>
#include <concepts>

namespace Helena::Util
{
    struct Function
    {
        template <typename Fn, typename... Args>
        [[nodiscard]] static constexpr auto BindFront(Fn&& fn, Args&&... args) noexcept {
            return [fn = std::forward<Fn>(fn), ...frontArgs = std::forward<Args>(args)](auto&&... args) constexpr -> decltype(auto)
            requires(std::invocable<Fn, Args..., decltype(args)...>) {
                return std::invoke(fn, frontArgs..., std::forward<decltype(args)>(args)...);
            };
        }

        template <typename Fn, typename... Args>
        [[nodiscard]] static constexpr auto BindBack(Fn&& fn, Args&&... args) noexcept {
            return [fn = std::forward<Fn>(fn), ...backArgs = std::forward<Args>(args)](auto&&... args) constexpr -> decltype(auto)
            requires(std::invocable<Fn, decltype(args)..., Args...>) {
                return std::invoke(fn, std::forward<decltype(args)>(args)..., backArgs...);
            };
        }
    };
}

#endif // HELENA_UTIL_FUNCTION_HPP