#ifndef HELENA_SYSTEMS_RESOURCEMANAGER_HPP
#define HELENA_SYSTEMS_RESOURCEMANAGER_HPP

#include <Helena/Types/VectorAny.hpp>

namespace Helena::Systems
{
    class ResourceManager final
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
        auto Create([[maybe_unused]] Args&&... args) -> void;

        template <typename Resource>
        auto Create(Resource&& instance) -> void;

        template <typename... Resources>
        [[nodiscard]] auto Has() const noexcept -> bool;

        template <typename... Resources>
        [[nodiscard]] auto Any() const noexcept -> bool;

        template <typename... Resources>
        [[nodiscard]] auto Get() noexcept -> decltype(auto);

        template <typename... Resources>
        [[nodiscard]] auto Get() const noexcept -> decltype(auto);

        template <typename... Resources>
        auto Remove() noexcept -> void;

    private:
        Types::VectorAny<UniqueKey, 64> m_Storage;
    };
}

namespace Helena::Events::ResourceManager
{
    template <typename Resource>
    struct Create {};

    template <typename Resource>
    struct Remove {};
}

#include <Helena/Systems/ResourceManager.ipp>

#endif // HELENA_SYSTEMS_RESOURCEMANAGER_HPP
