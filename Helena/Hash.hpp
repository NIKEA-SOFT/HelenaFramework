#ifndef HELENA_HASH_HPP
#define HELENA_HASH_HPP

#include <entt/core/type_info.hpp>
#include <entt/core/hashed_string.hpp>

namespace Helena
{
    namespace Hash {
        using String   = entt::hashed_string;
        using WString  = entt::hashed_wstring;

        template <typename T>
        constexpr auto Type = entt::type_hash<T>().value();
    }

    namespace Literals {
        using namespace entt::literals;
    }
}

#endif // HELENA_HASH_HPP
