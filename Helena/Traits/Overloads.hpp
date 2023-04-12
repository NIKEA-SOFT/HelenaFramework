#ifndef HELENA_TRAITS_OVERLOADS_HPP
#define HELENA_TRAITS_OVERLOADS_HPP

#include <type_traits>

namespace Helena::Traits {
    template <typename... Fn>
    struct Overloads : Fn... {
        using Fn::operator()...;
    };

    template <typename... Fn>
    Overloads(Fn&&...) -> Overloads<Fn...>;
}

#endif // HELENA_TRAITS_OVERLOADS_HPP