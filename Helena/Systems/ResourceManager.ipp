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

    template <typename Resource>
    auto ResourceManager::Create(Resource&& instance) -> void
    {
        using T = Traits::RemoveCVRefPtr<Resource>;
        if(!m_Storage.Has<T>()) {
            m_Storage.Create<T>(std::forward<Resource>(instance));
            Engine::SignalEvent<Events::ResourceManager::Create<T>>();
        }
    }

    template <typename... Resources>
    [[nodiscard]] bool ResourceManager::Has() const noexcept {
        return m_Storage.Has<Resources...>();
    }

    template <typename... Resources>
    [[nodiscard]] bool ResourceManager::Any() const noexcept {
        return m_Storage.Any<Resources...>();
    }

    template <typename... Resources>
    [[nodiscard]] decltype(auto) ResourceManager::Get() noexcept {
        return m_Storage.Get<Resources...>();
    }

    template <typename... Resources>
    [[nodiscard]] decltype(auto) ResourceManager::Get() const noexcept {
        return m_Storage.Get<Resources...>();
    }

    template <typename... Resources>
    void ResourceManager::Remove() noexcept 
    {
        if constexpr(sizeof...(Resources) == 1) {
            if(m_Storage.Has<Resources...>()) {
                Engine::SignalEvent<Events::ResourceManager::Remove<Resources...>>();
                m_Storage.Remove<Resources...>();
            }
        }
        else {
            (Remove<Resources>(), ...);
        }
    }
}

#endif // HELENA_SYSTEMS_RESOURCEMANAGER_IPP
