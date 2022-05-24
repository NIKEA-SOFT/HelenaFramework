#ifndef HELENA_TRAITS_SELECT_HPP
#define HELENA_TRAITS_SELECT_HPP

#include <type_traits>

namespace Helena::Traits {
    template<bool Condition, template <typename...> typename Success>
    struct Select {
        template <typename... T>
        using Args = Success<T...>;
    };

    template<template <typename...> typename Success>
    struct Select<false, Success> {
        template <typename... T>
        using Args = std::void_t<T...>;
    };
}

#endif // HELENA_TRAITS_SELECT_HPP