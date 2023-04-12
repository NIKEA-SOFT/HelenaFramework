// License: https://github.com/skypjack/entt

#ifndef HELENA_TYPES_DELEGATE_HPP
#define HELENA_TYPES_DELEGATE_HPP

#include <Helena/Platform/Assert.hpp>

#include <tuple>
#include <cstddef>
#include <utility>
#include <functional>
#include <type_traits>

namespace Helena::Types {

    /**
     * @brief Basic delegate implementation.
     *
     * Primary template isn't defined on purpose. All the specializations give a
     * compile-time error unless the template parameter is a function type.
     */
    template<typename = void>
    class Delegate {
    public:
        template<typename Ret, typename... Args>
        static auto function_pointer(Ret(*)(Args...)) -> Ret(*)(Args...);

        template<typename Ret, typename Type, typename... Args, typename Other>
        static auto function_pointer(Ret(*)(Type, Args...), Other &&) -> Ret(*)(Args...);

        template<typename Class, typename Ret, typename... Args, typename... Other>
        static auto function_pointer(Ret(Class:: *)(Args...), Other &&...) -> Ret(*)(Args...);

        template<typename Class, typename Ret, typename... Args, typename... Other>
        static auto function_pointer(Ret(Class:: *)(Args...) const, Other &&...) -> Ret(*)(Args...);

        template<typename Class, typename Type, typename... Other>
        static auto function_pointer(Type Class:: *, Other &&...) -> Type(*)();

    public:
        template<typename... Type>
        using function_pointer_t = decltype(function_pointer(std::declval<Type>()...));

        /*! @brief Used to wrap a function or a member of a specified type. */
        template<auto>
        struct arg_t {};

        /*! @brief Constant of type connect_arg_t used to disambiguate calls. */
        template<auto Func>
        static constexpr arg_t<Func> connect_arg{};
    };

    /**
     * @brief Utility class to use to send around functions and members.
     *
     * Unmanaged delegate for function pointers and members. Users of this class are
     * in charge of disconnecting instances before deleting them.
     *
     * A delegate can be used as a general purpose invoker without memory overhead
     * for free functions possibly with payloads and bound or unbound members.
     *
     * @tparam Ret Return type of a function type.
     * @tparam Args Types of arguments of a function type.
     */
    template<typename Ret, typename... Args>
    class Delegate<Ret(Args...)> {
        /**
        * @brief Transcribes the constness of a type to another type.
        * @tparam To The type to which to transcribe the constness.
        * @tparam From The type from which to transcribe the constness.
        */
        template<typename To, typename From>
        struct constness_as {
            /*! @brief The type resulting from the transcription of the constness. */
            using type = std::remove_const_t<To>;
        };


        /*! @copydoc constness_as */
        template<typename To, typename From>
        struct constness_as<To, const From> {
            /*! @brief The type resulting from the transcription of the constness. */
            using type = std::add_const_t<To>;
        };

        /**
        * @brief Alias template to facilitate the transcription of the constness.
        * @tparam To The type to which to transcribe the constness.
        * @tparam From The type from which to transcribe the constness.
        */
        template<typename To, typename From>
        using constness_as_t = typename constness_as<To, From>::type;

        /**
        * @brief A class to use to push around lists of types, nothing more.
        * @tparam Type Types provided by the type list.
        */
        template<typename... Type>
        struct type_list {
            /*! @brief Type list type. */
            using type = type_list;
            /*! @brief Compile-time number of elements in the type list. */
            static constexpr auto size = sizeof...(Type);
        };

        /*! @brief Primary template isn't defined on purpose. */
        template<std::size_t, typename>
        struct type_list_element;

        /**
        * @brief Provides compile-time indexed access to the types of a type list.
        * @tparam Index Index of the type to return.
        * @tparam Type First type provided by the type list.
        * @tparam Other Other types provided by the type list.
        */
        template<std::size_t Index, typename Type, typename... Other>
        struct type_list_element<Index, type_list<Type, Other...>>
            : type_list_element<Index - 1u, type_list<Other...>> {};

        /**
        * @brief Provides compile-time indexed access to the types of a type list.
        * @tparam Type First type provided by the type list.
        * @tparam Other Other types provided by the type list.
        */
        template<typename Type, typename... Other>
        struct type_list_element<0u, type_list<Type, Other...>> {
            /*! @brief Searched type. */
            using type = Type;
        };

        template<typename... Class, typename Ret_, typename... Args_>
        [[nodiscard]] constexpr auto index_sequence_for(Ret_(*)(Args_...)) {
            return std::index_sequence_for<Class..., Args_...>{};
        }

        /**
        * @brief Helper type.
        * @tparam Index Index of the type to return.
        * @tparam List Type list to search into.
        */
        template<std::size_t Index, typename List>
        using type_list_element_t = typename type_list_element<Index, List>::type;

        template<typename... Type>
        using function_pointer_t = typename Delegate<>::template function_pointer_t<Type...>;


        template<auto Candidate, std::size_t... Index>
        [[nodiscard]] auto wrap(std::index_sequence<Index...>) noexcept {
            return [](const void *, Args... args) -> Ret {
                [[maybe_unused]] const auto arguments = std::forward_as_tuple(std::forward<Args>(args)...);
                return static_cast<Ret>(std::invoke(Candidate, std::forward<type_list_element_t<Index, type_list<Args...>>>(std::get<Index>(arguments))...));
            };
        }

        template<auto Candidate, typename Type, std::size_t... Index>
        [[nodiscard]] auto wrap(Type &, std::index_sequence<Index...>) noexcept {
            return [](const void *payload, Args... args) -> Ret {
                [[maybe_unused]] const auto arguments = std::forward_as_tuple(std::forward<Args>(args)...);
                Type *curr = static_cast<Type *>(const_cast<constness_as_t<void, Type> *>(payload));
                return static_cast<Ret>(std::invoke(Candidate, *curr, std::forward<type_list_element_t<Index, type_list<Args...>>>(std::get<Index>(arguments))...));
            };
        }

        template<auto Candidate, typename Type, std::size_t... Index>
        [[nodiscard]] auto wrap(Type *, std::index_sequence<Index...>) noexcept {
            return [](const void *payload, Args... args) -> Ret {
                [[maybe_unused]] const auto arguments = std::forward_as_tuple(std::forward<Args>(args)...);
                Type *curr = static_cast<Type *>(const_cast<constness_as_t<void, Type> *>(payload));
                return static_cast<Ret>(std::invoke(Candidate, curr, std::forward<type_list_element_t<Index, type_list<Args...>>>(std::get<Index>(arguments))...));
            };
        }

    public:
        /*! @brief Function type of the contained target. */
        using function_type = Ret(const void *, Args...);
        /*! @brief Function type of the delegate. */
        using type = Ret(Args...);
        /*! @brief Return type of the delegate. */
        using result_type = Ret;

        template<auto Func>
        using arg_t = typename Delegate<>::template arg_t<Func>;

        /*! @brief Default constructor. */
        Delegate() noexcept
            : fn{nullptr}, data{nullptr}
        {}

        /**
         * @brief Constructs a delegate and connects a free function or an unbound
         * member.
         * @tparam Candidate Function or member to connect to the delegate.
         */
        template<auto Candidate>
        Delegate(arg_t<Candidate>) noexcept {
            Connect<Candidate>();
        }

        /**
         * @brief Constructs a delegate and connects a free function with payload or
         * a bound member.
         * @tparam Candidate Function or member to connect to the delegate.
         * @tparam Type Type of class or type of payload.
         * @param value_or_instance A valid object that fits the purpose.
         */
        template<auto Candidate, typename Type>
        Delegate(arg_t<Candidate>, Type &&value_or_instance) noexcept {
            Connect<Candidate>(std::forward<Type>(value_or_instance));
        }

        /**
         * @brief Constructs a delegate and connects an user defined function with
         * optional payload.
         * @param function Function to connect to the delegate.
         * @param payload User defined arbitrary data.
         */
        Delegate(function_type *function, const void *payload = nullptr) noexcept {
            Connect(function, payload);
        }

        /**
         * @brief Connects a free function or an unbound member to a delegate.
         * @tparam Candidate Function or member to connect to the delegate.
         */
        template<auto Candidate>
        void Connect() noexcept {
            data = nullptr;

            if constexpr(std::is_invocable_r_v<Ret, decltype(Candidate), Args...>) {
                fn = [](const void *, Args... args) -> Ret {
                    return Ret(std::invoke(Candidate, std::forward<Args>(args)...));
                };
            } else if constexpr(std::is_member_pointer_v<decltype(Candidate)>) {
                fn = wrap<Candidate>(index_sequence_for<type_list_element_t<0, type_list<Args...>>>(function_pointer_t<decltype(Candidate)>{}));
            } else {
                fn = wrap<Candidate>(index_sequence_for(function_pointer_t<decltype(Candidate)>{}));
            }
        }

        /**
         * @brief Connects a free function with payload or a bound member to a
         * delegate.
         *
         * The delegate isn't responsible for the connected object or the payload.
         * Users must always guarantee that the lifetime of the instance overcomes
         * the one of the delegate.<br/>
         * When used to connect a free function with payload, its signature must be
         * such that the instance is the first argument before the ones used to
         * define the delegate itself.
         *
         * @tparam Candidate Function or member to connect to the delegate.
         * @tparam Type Type of class or type of payload.
         * @param value_or_instance A valid reference that fits the purpose.
         */
        template<auto Candidate, typename Type>
        void Connect(Type &value_or_instance) noexcept {
            data = &value_or_instance;

            if constexpr(std::is_invocable_r_v<Ret, decltype(Candidate), Type &, Args...>) {
                fn = [](const void *payload, Args... args) -> Ret {
                    Type *curr = static_cast<Type *>(const_cast<constness_as_t<void, Type> *>(payload));
                    return Ret(std::invoke(Candidate, *curr, std::forward<Args>(args)...));
                };
            } else {
                fn = wrap<Candidate>(value_or_instance, index_sequence_for(function_pointer_t<decltype(Candidate), Type>{}));
            }
        }

        /**
         * @brief Connects a free function with payload or a bound member to a
         * delegate.
         *
         * @sa connect(Type &)
         *
         * @tparam Candidate Function or member to connect to the delegate.
         * @tparam Type Type of class or type of payload.
         * @param value_or_instance A valid pointer that fits the purpose.
         */
        template<auto Candidate, typename Type>
        void Connect(Type *value_or_instance) noexcept {
            data = value_or_instance;

            if constexpr(std::is_invocable_r_v<Ret, decltype(Candidate), Type *, Args...>) {
                fn = [](const void *payload, Args... args) -> Ret {
                    Type *curr = static_cast<Type *>(const_cast<constness_as_t<void, Type> *>(payload));
                    return Ret(std::invoke(Candidate, curr, std::forward<Args>(args)...));
                };
            } else {
                fn = wrap<Candidate>(value_or_instance, index_sequence_for(function_pointer_t<decltype(Candidate), Type>{}));
            }
        }

        /**
         * @brief Connects an user defined function with optional payload to a
         * delegate.
         *
         * The delegate isn't responsible for the connected object or the payload.
         * Users must always guarantee that the lifetime of an instance overcomes
         * the one of the delegate.<br/>
         * The payload is returned as the first argument to the target function in
         * all cases.
         *
         * @param function Function to connect to the delegate.
         * @param payload User defined arbitrary data.
         */
        void Connect(function_type *function, const void *payload = nullptr) noexcept {
            fn = function;
            data = payload;
        }

        /**
         * @brief Resets a delegate.
         *
         * After a reset, a delegate cannot be invoked anymore.
         */
        void Reset() noexcept {
            fn = nullptr;
            data = nullptr;
        }

        /**
         * @brief Returns the instance or the payload linked to a delegate, if any.
         * @return An opaque pointer to the underlying data.
         */
        [[nodiscard]] const void * GetData() const noexcept {
            return data;
        }

        /**
         * @brief Triggers a delegate.
         *
         * The delegate invokes the underlying function and returns the result.
         *
         * @warning
         * Attempting to trigger an invalid delegate results in undefined
         * behavior.
         *
         * @param args Arguments to use to invoke the underlying function.
         * @return The value returned by the underlying function.
         */
        Ret operator()(Args... args) const {
            HELENA_ASSERT(static_cast<bool>(*this), "Uninitialized delegate");
            return fn(data, std::forward<Args>(args)...);
        }

        /**
         * @brief Checks whether a delegate actually stores a listener.
         * @return False if the delegate is empty, true otherwise.
         */
        [[nodiscard]] operator bool() const noexcept {
            // no need to test also data
            return !(fn == nullptr);
        }

        /**
        * @brief Compares the contents of two delegates.
        * @tparam Ret Return type of a function type.
        * @tparam Args Types of arguments of a function type.
        * @param lhs A valid delegate object.
        * @param rhs A valid delegate object.
        * @return True if the two contents differ, false otherwise.
        */
        template<typename Ret_, typename... Args_>
        [[nodiscard]] bool operator!=(const Delegate<Ret_(Args_...)> &rhs) noexcept {
            return !(*this == rhs);
        }

        /**
         * @brief Compares the contents of two delegates.
         * @param other Delegate with which to compare.
         * @return False if the two contents differ, true otherwise.
         */
        [[nodiscard]] bool operator==(const Delegate<Ret(Args...)> &other) const noexcept {
            return fn == other.fn && data == other.data;
        }

    private:
        function_type *fn;
        const void *data;
    };

    /**
     * @brief Deduction guide.
     * @tparam Candidate Function or member to connect to the delegate.
     */
    template<auto Candidate>
    Delegate(typename Delegate<>::template arg_t<Candidate>) -> Delegate<std::remove_pointer_t<Delegate<>::function_pointer_t<decltype(Candidate)>>>;


    /**
     * @brief Deduction guide.
     * @tparam Candidate Function or member to connect to the delegate.
     * @tparam Type Type of class or type of payload.
     */
    //template<auto Candidate, typename Type>
    //Delegate(Delegate<...>::arg_t<Candidate>, Type &&) -> Delegate<std::remove_pointer_t<Internal::function_pointer_t<decltype(Candidate), Type>>>;

    template<auto Candidate, typename Type>
    Delegate(typename Delegate<>::template arg_t<Candidate>, Type &&)
        -> Delegate<std::remove_pointer_t<Delegate<>::function_pointer_t<decltype(Candidate), Type>>>;

    /**
     * @brief Deduction guide.
     * @tparam Ret Return type of a function type.
     * @tparam Args Types of arguments of a function type.
     */
    template<typename Ret, typename... Args>
    Delegate(Ret(*)(const void *, Args...), const void * = nullptr) -> Delegate<Ret(Args...)>;


}

#endif // HELENA_TYPES_DELEGATE_HPP