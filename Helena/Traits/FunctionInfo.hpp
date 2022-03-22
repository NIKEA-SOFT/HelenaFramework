#ifndef HELENA_TRAITS_FUNCTIONINFO_HPP
#define HELENA_TRAITS_FUNCTIONINFO_HPP

#include <functional>
#include <tuple>

namespace Helena::Traits {
    
    template <typename R>
    struct FunctionInfo;

    template <typename R, typename... Args>
    struct FunctionInfo<std::function<R(Args...)>> {
        using Ret = R;
        
        static constexpr auto ArgCount = sizeof...(Args);

        template <std::size_t index>
        struct Arg {
            using Type = typename std::tuple_element<index, std::tuple<Args...>>::type;
        };
    };
}

#endif // HELENA_TRAITS_FUNCTIONINFO_HPP