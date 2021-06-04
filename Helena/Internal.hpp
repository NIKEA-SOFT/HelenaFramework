#ifndef HELENA_INTERNAL_HPP
#define HELENA_INTERNAL_HPP

#include <Helena/Platform.hpp>

#include <entt/core/type_info.hpp>

#include <type_traits>
#include <string_view>

namespace Helena::Internal
{
    template <typename Type>
    constexpr auto NameOf = entt::type_name<Type>().value();

    template <typename Type>
    struct remove_cvref {
            using type = std::remove_cv_t<std::remove_reference_t<Type>>;
    };

    template <typename Type>
    using remove_cvref_t = typename remove_cvref<Type>::type;

    template <typename Type>
    using remove_cvrefptr_t = std::conditional_t<std::is_array_v<Type>, std::remove_all_extents_t<Type>,
        std::conditional_t<std::is_pointer_v<Type>, remove_cvref_t<std::remove_pointer_t<Type>>, remove_cvref_t<Type>>>;

    template <typename Default, typename AlwaysVoid, template<typename...> typename Op, typename... Args>
    struct detector {
            using value_t = std::false_type;
            using type = Default;
    };

    template <typename Default, template<typename...> typename Op, typename... Args>
    struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
            using value_t = std::true_type;
            using type = Op<Args...>;
    };

    struct nonesuch {
            nonesuch() = delete;
            ~nonesuch() = delete;
            nonesuch(nonesuch const&) = delete;
            void operator=(nonesuch const&) = delete;
    };

    template <template<typename...> typename Op, typename... Args>
    using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

    template <template<typename...> typename Op, typename... Args>
    constexpr bool is_detected_v = is_detected<Op, Args...>::value;

    template <template<typename...> typename Op, typename... Args>
    using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

    template <typename Default, template<typename...> typename Op, typename... Args>
    using detected_or = detector<Default, void, Op, Args...>;

    template <typename T, template <typename...> class Primary>
    struct is_specialization_of : std::false_type {};

    template <template <typename...> class Primary, typename... Args>
    struct is_specialization_of<Primary<Args...>, Primary> : std::true_type {};

    template <typename T, template <typename...> class Primary>
    constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;

    template <typename T, typename = void>
    struct is_pair : std::false_type {};

    template <typename T>
    struct is_pair<T, std::void_t<decltype(std::declval<typename T::first_type>()), decltype(std::declval<typename T::second_type>())>> : std::true_type {};

    template <typename T>
    constexpr bool is_pair_v = is_pair<T>::value;

    template<typename, typename = void>
    struct is_mapping : std::false_type {};

    template <typename Container>
    struct is_mapping<Container, std::enable_if_t<is_pair_v<typename std::iterator_traits<typename Container::iterator>::value_type>>> : std::true_type {};

    template <typename T>
    constexpr bool is_mapping_v = is_mapping<T>::value;

    template <typename T, bool B = std::is_enum<T>::value>
    struct is_scoped_enum : std::false_type {};

    template <typename T>
    struct is_scoped_enum<T, true> : std::integral_constant<bool, !std::is_convertible<T, typename std::underlying_type<T>::type>::value> {};

    template <typename T>
    constexpr bool is_scoped_enum_t = is_scoped_enum<T>::value;

    template <typename T>
    struct is_integral_constant : std::false_type {};

    template <typename T, T V>
    struct is_integral_constant<std::integral_constant<T, V>> : std::true_type {};

    template <typename T>
    constexpr bool is_integral_constant_v = is_integral_constant<T>::value;

#ifdef HF_STANDARD_CPP17
    template<class InputIt, class ForwardIt>
    [[nodiscard]] constexpr InputIt find_first_of(InputIt first, InputIt last, ForwardIt s_first, ForwardIt s_last) noexcept {
        for(; first != last; ++first) {
            for(ForwardIt it = s_first; it != s_last; ++it) {
                if(*first == *it) {
                    return first;
                }
            }
        }
        return last;
    }
#endif

    template <typename Container,
             typename Key = typename Container::key_type,
             typename Ret = typename Container::mapped_type,
             typename = std::enable_if_t<Internal::is_mapping_v<Internal::remove_cvref_t<Container>>, void>>
    auto AddOrGetTypeIndex(Container& container, const Key typeIndex) -> decltype(auto) {
        const auto [it, result] = container.emplace(typeIndex, container.size());
        return static_cast<Ret>(it->second);
    }
}

#endif // HELENA_INTERNAL_HPP
