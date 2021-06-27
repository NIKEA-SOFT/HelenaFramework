#ifndef HELENA_SYSTEMS_RESOURCEMANAGER_HPP
#define HELENA_SYSTEMS_RESOURCEMANAGER_HPP

#include <Helena/Internal.hpp>
#include <Helena/HashComparator.hpp>

#include <entt/core/any.hpp>
#include <robin_hood/robin_hood.h>

#include <vector>

namespace Helena::Systems
{
    class ResourceManager final
    {
        using map_index_t = robin_hood::unordered_flat_map<entt::id_type, std::size_t, Hash::Hasher<entt::id_type>, Hash::Comparator<entt::id_type>>;

        template <typename Resource>
        struct ResourceIndex {
            [[nodiscard]] static auto GetIndex(map_index_t& container) -> std::size_t;
        };

    public:
        ResourceManager() = default;
        ~ResourceManager() = default;
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) noexcept = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager& operator=(ResourceManager&&) noexcept = delete;

        template <typename Resource, typename... Args>
        auto Create([[maybe_unused]] Args&&... args) -> void;

        template <typename... Resources>
        [[nodiscard]] auto Has() noexcept -> bool;

        template <typename... Resources>
        [[nodiscard]] auto Any() noexcept -> bool;

        template <typename... Resources>
        [[nodiscard]] auto Get() noexcept -> decltype(auto);

        template <typename... Resources>
        auto Remove() noexcept -> void;

    private:
        std::vector<entt::any> m_Resources;
        map_index_t m_ResourceIndexes;
    };
}

namespace Helena::Events::Systems::ResourceManager
{
    template <typename Resource>
    struct Create {};

    template <typename Resource>
    struct Remove {};
}

#include <Helena/Systems/ResourceManager.ipp>

#endif // HELENA_SYSTEMS_RESOURCEMANAGER_HPP
