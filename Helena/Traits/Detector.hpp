#ifndef HELENA_TRAITS_DETECTOR_HPP
#define HELENA_TRAITS_DETECTOR_HPP

#include <type_traits>
#include <concepts>

namespace Helena::Traits
{
    namespace Details 
    {
        struct Nullable {
            Nullable() = delete;
            ~Nullable() = delete;
            Nullable(Nullable const&) = delete;
            void operator=(Nullable const&) = delete;
        };

        template <typename Default, typename, template <typename...> typename, typename...>
        struct Detector {
            using value_type = std::false_type;
            using storage_type = Default;
        };

        template <typename Default, template <typename...> typename Expression, typename... Args>
        struct Detector<Default, std::void_t<Expression<Args...>>, Expression, Args...> {
            using value_type = std::true_type;
            using storage_type = Expression<Args...>;
        };
    }

    template <template <typename...> typename Expression, typename... Args>
    using IsDetected = typename Details::Detector<Details::Nullable, void, Expression, Args...>::value_type;

    template <template <typename...> typename Expression, typename... Args>
    using DetectedType = typename Details::Detector<Details::Nullable, void, Expression, Args...>::storage_type;

    template <typename Default, template <typename...> typename Expression, typename... Args>
    using IsDetectedOr = typename Details::Detector<Default, void, Expression, Args...>::value_type;

    template <typename Default, template <typename...> typename Expression, typename... Args>
    using DetectedOrType = typename Details::Detector<Default, void, Expression, Args...>::storage_type;   

    template <template <typename...> typename Expression, typename... Args>
    concept Detected = IsDetected<Expression>::value;

    template <typename Default, template <typename...> typename Expression, typename... Args>
    concept DetectedOr = IsDetectedOr<Default, void, Expression, Args...>::value;
}


#endif // HELENA_TRAITS_DETECTOR_HPP