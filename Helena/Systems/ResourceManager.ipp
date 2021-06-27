#ifndef HELENA_SYSTEMS_RESOURCEMANAGER_IPP
#define HELENA_SYSTEMS_RESOURCEMANAGER_IPP

#include <Helena/Systems/ResourceManager.hpp>
#include <Helena/Core.hpp>

namespace Helena::Systems
{
    template <typename Resource>
    [[nodiscard]] auto ResourceManager::ResourceIndex<Resource>::GetIndex(map_index_t& container) -> std::size_t {
        static const std::size_t value = Internal::AddOrGetTypeIndex(container, Hash::Type<Resource>);
        return value;
    }

    template <typename Resource, typename... Args>
    auto ResourceManager::Create([[maybe_unused]] Args&&... args) -> void {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>,
                "Resource type cannot be const/ptr/ref");

        using Event = Helena::Events::Systems::ResourceManager::Create<Resource>;
        const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
        if(index >= m_Resources.size()) {
            m_Resources.resize(index + 1);
        }

        HF_ASSERT(!m_Resources[index], "Resource: {} already exist", Internal::NameOf<Resource>);
        m_Resources[index].template emplace<Resource>(std::forward<Args>(args)...);
        Core::TriggerEvent<Event>();
    }

    template <typename... Resources>
    [[nodiscard]] auto ResourceManager::Has() noexcept -> bool {
        static_assert(sizeof...(Resources) > 0, "Resource pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Resources>, Resources> && ...),
                "Resource type cannot be const/ptr/ref");

        if constexpr(sizeof...(Resources) == 1) {
            const auto index = ResourceIndex<Resources...>::GetIndex(m_ResourceIndexes);
            return index < m_Resources.size() && m_Resources[index];
        } else {
            return (Has<Resources>() && ...);
        }
    }

    template <typename... Resources>
    [[nodiscard]] auto ResourceManager::Any() noexcept -> bool {
        static_assert(sizeof...(Resources) > 1, "Exclusion-only Resource are not supported");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Resources>, Resources> && ...),
                "Resource type cannot be const/ptr/ref");

        return (Has<Resources>() || ...);
    }

    template <typename... Resources>
    [[nodiscard]] auto ResourceManager::Get() noexcept -> decltype(auto) {
        static_assert(sizeof...(Resources) > 0, "Resource pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Resources>, Resources> && ...),
                "Resource type cannot be const/ptr/ref");

        if constexpr(sizeof...(Resources) == 1) {
            const auto index = ResourceIndex<Resources...>::GetIndex(m_ResourceIndexes);
            HF_ASSERT(index < m_Resources.size() && m_Resources[index],
                    "Resource {} not exist", Internal::NameOf<Resources...>);
            return entt::any_cast<Resources&...>(m_Resources[index]);
        } else {
            return std::forward_as_tuple(Get<Resources>()...);
        }
    }

    template <typename... Resources>
    auto ResourceManager::Remove() noexcept -> void {
        static_assert(sizeof...(Resources) > 0, "Resource pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Resources>, Resources> && ...),
                "Resource type cannot be const/ptr/ref");

        if constexpr(sizeof...(Resources) == 1) {
            using Event = Helena::Events::Systems::ResourceManager::Remove<Resources...>;
            const auto index = ResourceIndex<Resources...>::GetIndex(m_ResourceIndexes);
            HF_ASSERT(index < m_Resources.size() && m_Resources[index],
                      "Resource {} not exist", Internal::NameOf<Resources...>);
            Core::TriggerEvent<Event>();
            m_Resources[index].reset();
        } else {
            (Remove<Resources>(), ...);
        }
    }
}

#endif // HELENA_SYSTEMS_RESOURCEMANAGER_IPP
