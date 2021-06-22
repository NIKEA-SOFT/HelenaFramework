#ifndef HELENA_HASHCOMPARATOR_HPP
#define HELENA_HASHCOMPARATOR_HPP

#include <entt/core/fwd.hpp>

namespace Helena::Hash
{
    template <typename T>
    struct Comparator {};

    template <typename T>
    struct Hasher {};

    template <>
    struct Comparator<entt::id_type> {
        using is_transparent = void;

        bool operator()(const entt::id_type lhs, const entt::id_type rhs) const noexcept {
            return lhs == rhs;
        }
    };

    template <>
    struct Hasher<entt::id_type> {
        std::size_t operator()(const entt::id_type key) const noexcept {
            return static_cast<std::size_t>(key);
        }
    };
}

#endif // HELENA_HASHCOMPARATOR_HPP
