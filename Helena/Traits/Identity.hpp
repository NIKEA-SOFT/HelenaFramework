#ifndef HELENA_TRAITS_IDENTITY_HPP
#define HELENA_TRAITS_IDENTITY_HPP

namespace Helena::Traits
{
    template <typename T>
    struct Identity {
        using Type = T;
    };
}


#endif // HELENA_TRAITS_IDENTITY_HPP