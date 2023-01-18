#ifndef HELENA_SYSTEMS_RESOURCEMANAGER_HPP
#define HELENA_SYSTEMS_RESOURCEMANAGER_HPP

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
    class ResourceManager final : public Types::ModernDesign<ResourceManager>
    {
        struct UniqueKey {};

    public:
        ResourceManager() = default;
        ~ResourceManager() = default;
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) noexcept = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager& operator=(ResourceManager&&) noexcept = delete;

        template <typename Resource, typename... Args>
        requires Traits::SameAs<Resource, Traits::RemoveCVRP<Resource>>
        static void Create(Args&&... args) {
            Engine::SignalEvent<Events::ResourceManager::PreCreate<Resource>>();
            CurrentSystem().m_Storage.template Create<Resource>(std::forward<Args>(args)...);
            Engine::SignalEvent<Events::ResourceManager::PostCreate<Resource>>(CurrentSystem().m_Storage.template Get<Resource>());
        }

        template <typename Resource>
        static void Create(Resource&& instance) {
            Create<Traits::RemoveCVRP<Resource>>(std::forward<Resource>(instance));
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
        static void Remove() {
            (Engine::SignalEvent<Events::ResourceManager::PreRemove<Resources>>(CurrentSystem().m_Storage.template Get<Resources>()), ...);
            CurrentSystem().m_Storage.template Remove<Resources...>();
            (Engine::SignalEvent<Events::ResourceManager::PostRemove<Resources>>(), ...);
        }

    private:
        Types::VectorAny<UniqueKey> m_Storage;
    };
}

#endif // HELENA_SYSTEMS_RESOURCEMANAGER_HPP