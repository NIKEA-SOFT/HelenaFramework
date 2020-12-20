#pragma once

#ifndef COMMON_HASH_HPP
#define COMMON_HASH_HPP

namespace Helena
{
    namespace Hash 
    {
        using HashString    = entt::hashed_string;
        using HashWString   = entt::hashed_wstring; 
        
        template <typename Type>
        using HashType      = entt::type_hash<Type>;

        namespace Literals {
            using namespace entt::literals;
        }
    }
}

#endif // COMMON_HASH_HPP