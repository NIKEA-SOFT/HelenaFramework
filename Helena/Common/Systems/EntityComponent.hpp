#ifndef COMMON_SYSTEMS_ENTITYCOMPONENT_HPP
#define COMMON_SYSTEMS_ENTITYCOMPONENT_HPP

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

        auto CreateEntity() -> Entity;

        template <typename Type, typename = std::enable_if_t<std::is_integral_v<Type>>>
        auto CreateEntity(const Type id) -> Entity;

        template <typename It>
        auto CreateEntity(It first, It last) -> void;

        auto HasEntity(const Entity id) const -> bool;

        auto SizeEntity() const -> std::size_t;

        auto AliveEntity() const -> std::size_t;

        auto ReserveEntity(const std::size_t size) -> void;

        auto RemoveEntity(const Entity id) -> void;

        template <typename It>
        auto RemoveEntity(It first, It last) -> void;

        static auto CastFromEntity(const Entity id);

        template <typename Type>
        static auto CastToEntity(const Type id) -> Entity;

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
        [[nodiscard]] auto HasComponent(const Entity id) -> bool;

        [[nodiscard]] auto HasComponents(const Entity id) -> bool;

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
        [[nodiscard]] auto SizeComponent() -> std::size_t;

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
    using System = Helena::Systems::EntityComponent;

    struct CreateEntity {
        System::Entity m_Entity {System::Null};
    };

    struct RemoveEntity {
        System::Entity m_Entity {System::Null};
    };

    template <typename Component>
    struct AddComponent {
        using Type = Internal::remove_cvrefptr_t<Component>;
        System::Entity m_Entity {System::Null};
    };

    template <typename Component>
    struct RemoveComponent {
        using Type = Internal::remove_cvrefptr_t<Component>;
        System::Entity m_Entity {System::Null};
    };

    struct RemoveComponents {
        System::Entity m_Entity {System::Null};
    };
}

#include <Common/Systems/EntityComponent.ipp>

#endif // COMMON_SYSTEMS_ENTITYCOMPONENT_HPP
