#ifndef __COMMON_HFTYPETRAITS_H__
#define __COMMON_HFTYPETRAITS_H__

#include <type_traits>

namespace Helena
{
    template <typename Base, typename Derived, typename = void>
    struct is_virtual_base_of : std::is_base_of<Base, Derived> {};

    template<typename Base, typename Derived>
    struct is_virtual_base_of
    <   Base, 
        Derived, 
        std::void_t<decltype(static_cast<Derived*>(std::declval<Base*>())), 
        std::enable_if_t<std::is_base_of_v<Base, Derived>>>
    > : std::false_type {};

    template<typename Base, typename Derived>
    inline constexpr bool is_virtual_base_of_v = is_virtual_base_of<Base, Derived>::value;
}

#endif // __COMMON_HFTYPETRAITS_H__