#ifndef HELENA_TRAITS_FUNCTIONINFO_HPP
#define HELENA_TRAITS_FUNCTIONINFO_HPP

#include <functional>

namespace Helena::Traits {
    
    template <typename>
    struct FunctionInfo;

    template <typename R, typename... Args>
    struct FunctionInfo<std::function<R(Args...)>> {
        using Ret = R;
        
        template <std::size_t index>
        struct Args {
            using Type = typename std::tuple_element<index, std::tuple<Args...>>::type;
        };
    };
}

#endif // HELENA_TRAITS_FUNCTIONINFO_HPP