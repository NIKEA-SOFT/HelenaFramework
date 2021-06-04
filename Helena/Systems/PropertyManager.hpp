#ifndef HELENA_SYSTEMS_PROPERTYMANAGER_HPP
#define HELENA_SYSTEMS_PROPERTYMANAGER_HPP

#include <Helena/Internal.hpp>
#include <Helena/HashComparator.hpp>

#include <entt/core/any.hpp>
#include <robin_hood/robin_hood.h>

#include <vector>

namespace Helena::Systems
{
    class PropertyManager final
    {
        using map_index_t = robin_hood::unordered_flat_map<entt::id_type, std::size_t, Hash::Hasher<entt::id_type>, Hash::Comparator<entt::id_type>>;

        template <typename Property>
        struct PropertyIndex {
            [[nodiscard]] static auto GetIndex(map_index_t& container) -> std::size_t;
        };

        template <typename Key, typename T>
        struct PropertyInfo {
            using Type = Key;
            using Value = T;
        };

    public:
        template <entt::id_type Key, typename Type,
        typename = std::enable_if_t<std::is_same_v<Type, Internal::remove_cvrefptr_t<Type>>>>
        using Property = PropertyInfo<entt::tag<Key>, Type>;

    public:
        PropertyManager() = default;
        ~PropertyManager() = default;
        PropertyManager(const PropertyManager&) = delete;
        PropertyManager(PropertyManager&&) noexcept = delete;
        PropertyManager& operator=(const PropertyManager&) = delete;
        PropertyManager& operator=(PropertyManager&&) noexcept = delete;

        template <typename Property, typename... Args>
        auto Create([[maybe_unused]] Args&&... args) -> void;

        template <typename... Property>
        [[nodiscard]] auto Has() noexcept -> bool;

        template <typename... Property>
        [[nodiscard]] auto Any() noexcept -> bool;

        template <typename... Property>
        [[nodiscard]] auto Get() noexcept -> decltype(auto);

        template <typename... Property>
        auto Remove() noexcept -> void;

    private:
        std::vector<entt::any> m_Properties;
        map_index_t m_PropertyIndexes;
    };
}

namespace Helena::Events::Systems::Property
{
    template <typename Key>
    struct Create {};

    template <typename Key>
    struct Remove {};
}

#include <Helena/Systems/PropertyManager.ipp>

#endif // HELENA_SYSTEMS_PROPERTYMANAGER_HPP
