#ifndef HELENA_TRAITS_TYPECOUNTER_HPP
#define HELENA_TRAITS_TYPECOUNTER_HPP

namespace Helena::Traits
{
    template <typename... T>
    inline constexpr auto TypeCounter = sizeof...(T);
}

#endif // HELENA_TRAITS_TYPECOUNTER_HPP
