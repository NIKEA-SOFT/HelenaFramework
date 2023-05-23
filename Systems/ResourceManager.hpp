#ifndef HELENA_SYSTEMS_RESOURCEMANAGER_HPP
#define HELENA_SYSTEMS_RESOURCEMANAGER_HPP

#include <Helena/Types/System.hpp>
#include <Helena/Types/VectorAny.hpp>

namespace Helena::Events::ResourceManager
{
    template <typename Resource>
    struct PreCreate {};

    template <typename Resource>
    struct PostCreate {
        Resource& resource;
    };

    template <typename Resource>
    struct PreRemove {
        Resource& resource;
    };

    template <typename Resource>
    struct PostRemove {};
}

namespace Helena::Systems
{
    class ResourceManager final : Types::System<ResourceManager>
    {
        struct UniqueKey {};

    public:
        ResourceManager() = default;
        ~ResourceManager() = default;
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) noexcept = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager& operator=(ResourceManager&&) noexcept = delete;

        template <typename Resource>
        static void Create(decltype(Engine::NoSignal), Resource&& instance) {
            CurrentSystem().m_Storage.template Create<Traits::RemoveCVRP<Resource>>(std::forward<Resource>(instance));
        }

        template <typename Resource, typename... Args>
        requires Traits::SameAs<Resource, Traits::RemoveCVRP<Resource>>
        static void Create(decltype(Engine::NoSignal), Args&&... args) {
            CurrentSystem().m_Storage.template Create<Resource>(std::forward<Args>(args)...);
        }

        template <typename Resource>
        static void Create(Resource&& instance) {
            auto& currentSystem = CurrentSystem();
            Engine::SignalEvent<Events::ResourceManager::PreCreate<Traits::RemoveCVRP<Resource>>>();
            currentSystem.m_Storage.template Create<Traits::RemoveCVRP<Resource>>(std::forward<Resource>(instance));
            Engine::SignalEvent<Events::ResourceManager::PostCreate<Traits::RemoveCVRP<Resource>>>(
                currentSystem.m_Storage.template Get<Traits::RemoveCVRP<Resource>>());
        }

        template <typename Resource, typename... Args>
        requires Traits::SameAs<Resource, Traits::RemoveCVRP<Resource>>
        static void Create(Args&&... args) {
            auto& currentSystem = CurrentSystem();
            Engine::SignalEvent<Events::ResourceManager::PreCreate<Resource>>();
            currentSystem.m_Storage.template Create<Resource>(std::forward<Args>(args)...);
            Engine::SignalEvent<Events::ResourceManager::PostCreate<Resource>>(currentSystem.m_Storage.template Get<Resource>());
        }

        template <typename... Resources>
        requires (Traits::SameAs<Resources, Traits::RemoveCVRP<Resources>> && ...)
        [[nodiscard]] static bool Has() noexcept {
            return CurrentSystem().m_Storage.template Has<Resources...>();
        }

        template <typename... Resources>
        requires (Traits::SameAs<Resources, Traits::RemoveCVRP<Resources>> && ...)
        [[nodiscard]] static bool Any() noexcept {
            return CurrentSystem().m_Storage.template Any<Resources...>();
        }

        template <typename... Resources>
        requires (Traits::SameAs<Resources, Traits::RemoveCVRP<Resources>> && ...)
        [[nodiscard]] static decltype(auto) Get() noexcept {
            return CurrentSystem().m_Storage.template Get<Resources...>();
        }

        template <typename... Resources>
        requires (Traits::SameAs<Resources, Traits::RemoveCVRP<Resources>> && ...)
        static void Remove(decltype(Engine::NoSignal)) {
            CurrentSystem().m_Storage.template Remove<Resources...>();
        }

        template <typename... Resources>
        requires (Traits::SameAs<Resources, Traits::RemoveCVRP<Resources>> && ...)
        static void Remove() {
            auto& currentSystem = CurrentSystem();
            (Engine::SignalEvent<Events::ResourceManager::PreRemove<Resources>>(currentSystem.m_Storage.template Get<Resources>()), ...);
            currentSystem.m_Storage.template Remove<Resources...>();
            (Engine::SignalEvent<Events::ResourceManager::PostRemove<Resources>>(), ...);
        }

    private:
        Types::VectorAny<UniqueKey> m_Storage;
    };
}

#endif // HELENA_SYSTEMS_RESOURCEMANAGER_HPP