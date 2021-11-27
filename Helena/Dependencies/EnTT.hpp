#ifndef HELENA_DEPENDENCIES_ENTT_HPP
#define HELENA_DEPENDENCIES_ENTT_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Types/Hash.hpp>

#ifdef ENTT_ASSERT
    #undef ENTT_ASSERT
#endif

#define ENTT_ASSERT     HELENA_ASSERT
#define ENTT_ID_TYPE    std::uint32_t

#include <Dependencies/entt/entt.hpp>

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

#endif // HELENA_DEPENDENCIES_ENTT_HPP