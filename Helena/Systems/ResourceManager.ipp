#ifndef HELENA_SYSTEMS_RESOURCEMANAGER_IPP
#define HELENA_SYSTEMS_RESOURCEMANAGER_IPP

#include <Helena/Systems/ResourceManager.hpp>
#include <Helena/Engine/Engine.hpp>

namespace Helena::Systems
{
    template <typename Resource, typename... Args>
    auto ResourceManager::Create([[maybe_unused]] Args&&... args) -> void 
    {
        if(!m_Storage.Has<Resource>()) {
            m_Storage.Create<Resource>(std::forward<Args>(args)...);
            Engine::SignalEvent<Events::ResourceManager::Create<Resource>>();
        }
    }

    template <typename... Resources>
    [[nodiscard]] bool ResourceManager::Has() noexcept {
        return m_Storage.Has<Resources...>();
    }

    template <typename... Resources>
    [[nodiscard]] bool ResourceManager::Any() noexcept {
        return m_Storage.Any<Resources...>();
    }

    template <typename... Resources>
    [[nodiscard]] decltype(auto) ResourceManager::Get() noexcept {
        return m_Storage.Get<Resources...>();
    }

    template <typename... Resources>
    void ResourceManager::Remove() noexcept 
    {
        if(m_Storage.Has<Resources...>()) {
            m_Storage.Remove<Resources...>();
        }
    }
}

#endif // HELENA_SYSTEMS_RESOURCEMANAGER_IPP
