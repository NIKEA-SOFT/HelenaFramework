#pragma once

#include <Helena/Debug/Assert.hpp>
#include <Helena/Types/Hash.hpp>

#ifdef ENTT_ASSERT
    #undef ENTT_ASSERT
    #define ENTT_ASSERT HF_ASSERT
#endif

#define ENTT_ID_TYPE std::uint32_t

#include <entt/entt.hpp>

namespace Helena::Hash
{
    template <>
    struct Hasher<entt::id_type> {
        std::size_t operator()(const entt::id_type key) const noexcept {
            return static_cast<std::size_t>(key);
        }
    };

    template <>
    struct Equaler<entt::id_type> {
        bool operator()(const entt::id_type lhs, const entt::id_type rhs) const noexcept {
            return lhs == rhs;
        }
    };
}
