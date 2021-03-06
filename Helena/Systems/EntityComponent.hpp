#ifndef HELENA_SYSTEMS_ENTITYCOMPONENT_HPP
#define HELENA_SYSTEMS_ENTITYCOMPONENT_HPP

#include <entt/entity/registry.hpp>

#include <Helena/Internal.hpp>

namespace Helena::Systems
{
    class EntityComponent final
    {
    public:
        static constexpr auto Null = entt::null;

        using Entity = entt::entity;

        template <entt::id_type Value>
        using Tag = entt::tag<Value>;

        template <typename... Type>
        using ExcludeType = entt::exclude_t<Type...>;

        template <typename... Type>
        using GetType = entt::get_t<Type...>;

        template<typename... Type>
        static constexpr ExcludeType<Type...> Exclude{};

        template<typename... Type>
        static constexpr GetType<Type...> Get{};

    public:
        EntityComponent() = default;
        ~EntityComponent();
        EntityComponent(const EntityComponent&) = delete;
        EntityComponent(EntityComponent&&) noexcept = delete;
        EntityComponent& operator=(const EntityComponent&) = delete;
        EntityComponent& operator=(EntityComponent&&) noexcept = delete;

        [[nodiscard]] auto CreateEntity() -> Entity;

        template <typename Type, typename = std::enable_if_t<std::is_integral_v<Type>>>
        [[nodiscard]] auto CreateEntity(const Type id) -> Entity;

        template <typename It>
        auto CreateEntity(It first, It last) -> void;

        [[nodiscard]] auto HasEntity(const Entity id) const -> bool;

        [[nodiscard]] auto SizeEntity() const noexcept -> std::size_t;

        [[nodiscard]] auto AliveEntity() const noexcept -> std::size_t;

        auto ReserveEntity(const std::size_t size) -> void;

        auto RemoveEntity(const Entity id) -> void;

        template <typename It>
        auto RemoveEntity(It first, It last) -> void;

        template <typename Type, typename = std::enable_if_t<std::is_integral_v<Type>>>
        [[nodiscard]] static auto Cast(const Type id) noexcept;

        [[nodiscard]] static auto Cast(const Entity id) noexcept;

        template <typename Func>
        auto Each(Func&& callback) const -> void;

        template <typename Func>
        auto EachOrphans(Func&& callback) const -> void;

        template <typename Component, typename... Args>
        auto AddComponent(const Entity id, Args&&... args) -> void;

        //template <std::size_t Type, std::integral_constant<typename Type> Component>
        //auto AddComponentTag(const Entity id) -> void;

        //template <typename Component, typename... Args>
        //auto AddOrGetComponent(const Entity id, Args&&... args) -> Component&;

        //template <typename Component, typename... Args>
        //auto AddOrReplaceComponent(const Entity id, Args&&... args) -> Component&;

        template <typename Func>
        auto VisitComponent(const Entity id, Func&& callback) const -> void;

        template <typename Func>
        auto VisitComponent(Func&& callback) const -> void;

        template <typename... Components>
        [[nodiscard]] auto GetComponent(const Entity id) -> decltype(auto);

        template <typename... Components>
        [[nodiscard]] auto GetComponent(const Entity id) const -> decltype(auto);

        template <typename... Components>
        [[nodiscard]] auto GetComponentPtr(const Entity id);

        template <typename... Components>
        [[nodiscard]] auto GetComponentPtr(const Entity id) const;

        template <typename... Components>
        [[nodiscard]] auto HasComponent(const Entity id) const -> bool;

        [[nodiscard]] auto HasComponent(const Entity id) const -> bool;

        template <typename... Components>
        [[nodiscard]] auto AnyComponent(const Entity id) const -> bool;

        template <typename... Components, typename... ExcludeFilter>
        [[nodiscard]] auto ViewComponent(ExcludeType<ExcludeFilter...> = {}) -> decltype(auto);

        template <typename... Components, typename... ExcludeFilter>
        [[nodiscard]] auto ViewComponent(ExcludeType<ExcludeFilter...> = {}) const -> decltype(auto);

        template <typename... Owned, typename... ExcludeFilter>
        [[nodiscard]] auto GroupComponent(ExcludeType<ExcludeFilter...> = {}) -> decltype(auto);

        template <typename... Owned, typename... ExcludeFilter>
        [[nodiscard]] auto GroupComponent(ExcludeType<ExcludeFilter...> = {}) const -> decltype(auto);

        template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
        [[nodiscard]] auto GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...> = {}) -> decltype(auto);

        template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
        [[nodiscard]] auto GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...> = {}) const -> decltype(auto);

        template <typename... Components>
        auto RemoveComponent(const Entity id) -> void;

        template <typename... Components, typename It>
        auto RemoveComponent(It first, It last) -> void;

        auto RemoveComponents(const Entity id) -> void;

        template <typename... Components>
        auto ClearComponent() -> void;

        auto Clear() -> void;

        template <typename Component>
        [[nodiscard]] auto SizeComponent() const -> std::size_t;

        template <typename... Components>
        auto ReserveComponent(const std::size_t size) -> void;

        auto ReservePool(const std::size_t size) -> void;

    private:
        entt::registry m_Registry;
    };
}

namespace Helena::Events::Systems::EntityComponent
{
    struct CreateEntity {
        Helena::Systems::EntityComponent::Entity m_Entity {Helena::Systems::EntityComponent::Null};
    };

    struct RemoveEntity {
        Helena::Systems::EntityComponent::Entity m_Entity {Helena::Systems::EntityComponent::Null};
    };

    template <typename Component>
    struct AddComponent {
        Helena::Systems::EntityComponent::Entity m_Entity {Helena::Systems::EntityComponent::Null};
    };

    template <typename Component>
    struct RemoveComponent {
        Helena::Systems::EntityComponent::Entity m_Entity {Helena::Systems::EntityComponent::Null};
    };

    struct RemoveComponents {
        Helena::Systems::EntityComponent::Entity m_Entity {Helena::Systems::EntityComponent::Null};
    };
}

#include <Helena/Systems/EntityComponent.ipp>

#endif // HELENA_SYSTEMS_ENTITYCOMPONENT_HPP
