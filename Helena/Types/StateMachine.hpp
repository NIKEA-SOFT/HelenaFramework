#ifndef HELENA_TYPES_STATEMACHINE_HPP
#define HELENA_TYPES_STATEMACHINE_HPP

#include <Helena/Traits/AnyOf.hpp>
#include <Helena/Traits/Overloads.hpp>
#include <Helena/Traits/UniqueTypes.hpp>

#include <variant>
#include <utility>

namespace Helena::Types
{
    template <typename... States>
    requires (std::is_class_v<States> && ...)
    class StateMachine
    {
        struct NoState {
            void operator()(const NoState&) const noexcept {}
        };

        template <typename... T>
        using FSMStates = std::variant<NoState, States...>;

        template <typename Dispatcher>
        requires (std::is_invocable_v<Dispatcher, StateMachine&, States&> && ...)
        constexpr void VisitOverloads() const {
            std::visit([self = const_cast<StateMachine*>(this)](auto& state) {
                if constexpr(!std::is_same_v<std::decay_t<decltype(state)>, NoState>) {
                    static_assert(std::is_invocable_v<Dispatcher, StateMachine&, decltype(state)>,
                        "Dispatcher does not support the operator(StateMachine<States...>&, State&)");
                    Dispatcher{}(*self, state);
                }
            }, const_cast<FSMStates<States...>&>(m_States));
        }

        template <typename... Fn>
        static constexpr auto UniqueFunctions = Traits::UniqueTypes<Fn...>;

        template <typename Fn>
        static constexpr auto Invocable = (std::invocable<Fn, States> || ...);

    public:
        StateMachine() = default;
        ~StateMachine() = default;
        StateMachine(const StateMachine&) = default;
        StateMachine(StateMachine&&) noexcept = default;
        StateMachine& operator=(const StateMachine&) = default;
        StateMachine& operator=(StateMachine&&) noexcept = default;

        template <typename State, typename... Args>
        requires (Traits::AnyOf<State, States...> && std::is_constructible_v<State, Args...>)
        constexpr void SetState(Args&&... args) noexcept(std::is_nothrow_constructible_v<State, Args...>) {
            m_States.template emplace<State>(std::forward<Args>(args)...);
        }

        template <typename State>
        requires (Traits::AnyOf<State, States...> && std::is_constructible_v<std::decay_t<State>, State>)
        constexpr void SetState(State&& state) noexcept(std::is_nothrow_constructible_v<std::decay_t<State>, State>) {
            m_States.template emplace<std::decay_t<State>>(std::forward<State>(state));
        }

        template <typename... Fn>
        requires (UniqueFunctions<Fn...> && (Invocable<Fn> && ...))
        constexpr void Visit(Fn&&... visitor) {
            std::visit(Traits::Overloads{NoState{}, std::forward<Fn>(visitor)...}, m_States);
        }

        template <typename T>
        requires Traits::AnyOf<T, States...>
        [[nodiscard]] constexpr auto GetIf() const noexcept {
            return std::get_if<T>(&m_States);
        }

        template <typename T>
        requires Traits::AnyOf<T, States...>
        [[nodiscard]] constexpr auto GetIf() noexcept {
            return std::get_if<T>(&m_States);
        }

        template <typename State>
        requires Traits::AnyOf<State, States...>
        [[nodiscard]] constexpr bool HasState() const noexcept {
            return std::holds_alternative<State>(m_States);
        }

        [[nodiscard]] constexpr bool HasState() const noexcept {
            return !std::holds_alternative<NoState>(m_States);
        }

        template <typename... State>
        requires (sizeof...(State) > 1 && (Traits::AnyOf<State, States...> && ...))
        [[nodiscard]] constexpr bool AnyState() const noexcept {
            return (HasState<State>() || ...);
        }

        constexpr void ResetState() noexcept {
            m_States.template emplace<NoState>();
        }

        template <typename Dispatcher>
        requires (std::is_default_constructible_v<Dispatcher>) && (std::is_invocable_v<Dispatcher, StateMachine&, States&> && ...)
        constexpr void Dispatch() const {
            VisitOverloads<Dispatcher>();
        }

        template <typename Dispatcher, typename NewState, typename... Args>
        requires (std::is_default_constructible_v<Dispatcher>)
              && (std::is_invocable_v<Dispatcher, StateMachine&, States&> && ...)
              && (Traits::AnyOf<NewState, States...> && std::is_constructible_v<NewState, Args...>)
        constexpr void Dispatch(Args&&... args) {
            SetState<NewState>(std::forward<Args>(args)...);
            VisitOverloads<Dispatcher>();
        }

        template <typename Dispatcher, typename State>
        requires (std::is_default_constructible_v<Dispatcher>)
              && (std::is_invocable_v<Dispatcher, StateMachine&, States&> && ...)
              && (Traits::AnyOf<State, States...> && std::is_constructible_v<std::decay_t<State>, State>)
        constexpr void Dispatch(State&& newState) {
            SetState(std::forward<State>(newState));
            VisitOverloads<Dispatcher>();
        }

    private:
        FSMStates<States...> m_States;
    };
}

#endif // HELENA_TYPES_STATEMACHINE_HPP