#pragma once

#ifndef COMMON_HASH_HPP
#define COMMON_HASH_HPP

namespace Helena
{
    namespace Hash 
    {
        using String   = entt::hashed_string;
        using WString  = entt::hashed_wstring; 

        template <typename T>
        constexpr auto Type = entt::type_hash<T>().value();

        namespace Literals {
            using namespace entt::literals;
        }
    }
}

#endif // COMMON_HASH_HPP