#ifndef HELENA_TYPES_OVERLOADS_HPP
#define HELENA_TYPES_OVERLOADS_HPP

#include <type_traits>

namespace Helena::Types
{
    template <typename... Fn>
    struct Overloads : Fn... {
        using Fn::operator()...;
    };

    template <typename... Fn>
    Overloads(Fn&&...) -> Overloads<std::remove_cvref_t<Fn>...>;
}

#endif // HELENA_TYPES_OVERLOADS_HPP