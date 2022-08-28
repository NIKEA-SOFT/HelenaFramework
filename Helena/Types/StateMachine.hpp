#ifndef HELENA_TYPES_STATEMACHINE_HPP
#define HELENA_TYPES_STATEMACHINE_HPP

#include <array>
#include <variant>
#include <Helena/Traits/AnyOf.hpp>
#include <Helena/Traits/Specialization.hpp>

namespace Helena::Types
{
    class StateMachine
    {
        template <typename Dispatcher, typename States, std::size_t... Indexes>
        static constexpr auto GenerateOverloads(std::index_sequence<Indexes...>) noexcept {
            static_assert(std::is_same_v<std::remove_cvref_t<States>, States>, "States type incorrect!");
            return std::array{+[]([[maybe_unused]] std::add_lvalue_reference_t<States> fsm) {
                if constexpr(Indexes > 0) {
                    Dispatcher{}(fsm, std::get<Indexes>(fsm));
                }
            }...};
        }

    public:
        struct NoState {};

        template <typename... T>
        using States = std::variant<NoState, T...>;

    public:
        StateMachine() noexcept = delete;
        ~StateMachine() noexcept = delete;
        StateMachine(const StateMachine&) = delete;
        StateMachine(StateMachine&&) noexcept = delete;
        StateMachine& operator=(const StateMachine&) = delete;
        StateMachine& operator=(StateMachine&&) noexcept = delete;

        template <typename State, typename... T, typename... Args>
        requires Traits::AnyOf<State, T...>
        static constexpr void ChangeState(States<T...>& fsm, [[maybe_unused]] Args&&... args) noexcept(noexcept(
            fsm.emplace<State>(std::forward<Args>(args)...))) {
            fsm.emplace<State>(std::forward<Args>(args)...);
        }

        template <typename State, typename... T>
        requires Traits::AnyOf<State, T...>
        [[nodiscard]] static constexpr bool CurrentState(States<T...>& fsm) noexcept {
            return std::holds_alternative<State>(fsm);
        }

        template <typename... T>
        static constexpr void ResetState(States<T...>& fsm) noexcept {
            fsm.emplace<NoState>();
        }

        template <typename Dispatcher, typename... T>
        static constexpr void Dispatch(States<T...>& fsm) {
            std::visit([&fsm](auto& state) {
                if constexpr(!std::is_same_v<std::remove_cvref_t<decltype(state)>, NoState>) {
                    static_assert(std::is_invocable_v<Dispatcher, decltype(fsm), decltype(state)>,
                        "Dispatcher does not support the operator() or state is not lvalue reference");
                    Dispatcher{}(fsm, state);
                }
            }, fsm);
        }
    };
}

#endif // HELENA_TYPES_STATEMACHINE_HPP