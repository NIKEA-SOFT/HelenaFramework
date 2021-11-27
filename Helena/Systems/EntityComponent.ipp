#ifndef HELENA_SYSTEMS_ENTITYCOMPONENT_IPP
#define HELENA_SYSTEMS_ENTITYCOMPONENT_IPP

#include <Helena/Systems/EntityComponent.hpp>
#include <Helena/Engine/Engine.hpp>
#include <Helena/Traits/IntegralConstant.hpp>

namespace entt
{
    template <typename Type>
    struct ENTT_API type_seq<Type>
    {
        [[nodiscard]] inline static id_type value() {
            static const auto value = Helena::Engine::GetSystem<Helena::Systems::EntityComponent>().GetSequenceIndex(Helena::Hash::Get<Type>());
            return value;
        }
    };
}

namespace Helena::Systems
{
    inline EntityComponent::~EntityComponent() {
        Clear();
    }

    [[nodiscard]] inline auto EntityComponent::CreateEntity() -> Entity
    {
        const auto entity = m_Registry.create();
        Engine::SignalEvent<Events::EntityComponent::CreateEntity>(entity);
        return entity;
    }

    template <typename Type, typename>
    [[nodiscard]] auto EntityComponent::CreateEntity(const Type id) -> Entity
    {
        HELENA_ASSERT(!HasEntity(static_cast<const Entity>(id)), "Entity {} is already exist", id);
        const auto entity = m_Registry.create(static_cast<const entt::entity>(id));
        Engine::SignalEvent<Events::EntityComponent::CreateEntity>(entity);

        return entity;
    }

    template<typename It>
    auto EntityComponent::CreateEntity(It first, It last) -> void 
    {
        m_Registry.create(first, last);
        for(; first != last; ++first) {
            Engine::SignalEvent<Events::EntityComponent::CreateEntity>(*first);
        }
    }

    [[nodiscard]] inline auto EntityComponent::HasEntity(const Entity id) const -> bool {
        return m_Registry.valid(id);
    }

    [[nodiscard]] inline auto EntityComponent::SizeEntity() const noexcept -> std::size_t {
        return m_Registry.size();
    }

    [[nodiscard]] inline auto EntityComponent::AliveEntity() const -> std::size_t {
        return m_Registry.alive();
    }

    inline auto EntityComponent::ReserveEntity(const std::size_t size) -> void {
        m_Registry.reserve(size);
    }

    inline auto EntityComponent::RemoveEntity(const Entity id) -> void
    {
        HELENA_ASSERT(HasEntity(id), "Entity {} is not valid", id);
        Engine::SignalEvent<Events::EntityComponent::RemoveEntity>(id);
        m_Registry.destroy(id);
    }

    template<typename It>
    auto EntityComponent::RemoveEntity(It first, It last) -> void
    {
        for(; first != last; ++first) {
            HELENA_ASSERT(HasEntity(*first), "Entity {} is not valid", *first);
            Engine::SignalEvent<Events::EntityComponent::RemoveEntity>(*first);
            m_Registry.destroy(*first);
        }
    }

    template <typename Type, typename>
    [[nodiscard]] auto EntityComponent::Cast(const Type id) noexcept {
        static_assert(std::is_same_v<Traits::RemoveCVRefPtr<Type>, Type>, "Component type cannot be const/ptr/ref");
        return static_cast<std::underlying_type_t<Entity>>(id);
    }

    [[nodiscard]] inline auto EntityComponent::Cast(const Entity id) noexcept {
        return entt::to_integral(id);
    }

    template <typename Func>
    auto EntityComponent::Each(Func&& callback) const -> void {
        static_assert(std::is_invocable_v<Func>, "Callback is not a callable type");
        m_Registry.each(std::forward<Func>(callback));
    }

    template <typename Func>
    auto EntityComponent::EachOrphans(Func&& callback) const -> void {
        static_assert(std::is_invocable_v<Func>, "Callback is not a callable type");
        m_Registry.orphans(std::forward<Func>(callback));
    }

    // Components
    template <typename Component, typename... Args>
    auto EntityComponent::AddComponent(const Entity id, Args&&... args) -> decltype(auto)
    {
        static_assert(std::is_same_v<Traits::RemoveCVRefPtr<Component>, Component>, "Component type cannot be const/ptr/ref");

        HELENA_ASSERT(HasEntity(id), "Entity {} is not valid", id);
        HELENA_ASSERT(!HasComponent<Component>(id), "Entity {}, component {} is already exist", id, Traits::NameOf<Component>::value);

        auto& component = m_Registry.emplace<Component>(id, std::forward<Args>(args)...);
        Engine::SignalEvent<Events::EntityComponent::AddComponent<Component>>(id);
        return component;
    }

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::GetComponent(const Entity id) -> decltype(auto)
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert(((!Traits::IsIntegralConstant<Components>::value) && ...), "This method not support tags!");

        HELENA_ASSERT(HasEntity(id), "Entity {} not valid", id);
        HELENA_ASSERT(HasComponent<Components...>(id), "Entity id {} one of the components is not exist!", id);
        return m_Registry.get<Components...>(id);
    }

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::GetComponent(const Entity id) const -> decltype(auto)
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert(((!Traits::IsIntegralConstant<Components>::value) && ...), "This method not support tags!");

        HELENA_ASSERT(HasEntity(id), "Entity {} not valid", id);
        HELENA_ASSERT(HasComponent<Components...>(id), "Entity {} one of the components is not exist!", id);

        return m_Registry.get<Components...>(id);
    }

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::GetComponentPtr(const Entity id)
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert(((!Traits::IsIntegralConstant<Components>::value) && ...), "This method not support tags!");

        HELENA_ASSERT(HasEntity(id), "Entity {} not valid", id);
        return m_Registry.try_get<Components...>(id);
    }

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::GetComponentPtr(const Entity id) const
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert(((!Traits::IsIntegralConstant<Components>::value) && ...), "This method not support tags!");

        HELENA_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
        return m_Registry.try_get<Components...>(id);
    }

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::HasComponent(const Entity id) const -> bool
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

        HELENA_ASSERT(HasEntity(id), "Entity {} not valid", id);
        return m_Registry.all_of<Components...>(id);
    }

    [[nodiscard]] auto EntityComponent::HasComponent(const Entity id) const -> bool {
        return !m_Registry.orphan(id);
    }

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::AnyComponent(const Entity id) const -> bool
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

        return m_Registry.any_of<Components...>(id);
    }

    template <typename Func>
    auto EntityComponent::VisitComponent(const Entity id, Func&& callback) const -> void
    {
        static_assert(std::is_invocable_v<Func>, "Callback is not a callable type");

        HELENA_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
        m_Registry.visit(id, std::forward<Func>(callback));
    }

    template <typename Func>
    auto EntityComponent::VisitComponent(Func&& callback) const -> void {
        static_assert(std::is_invocable_v<Func>, "Callback is not a callable type");
        m_Registry.visit(std::forward<Func>(callback));
    }

    template <typename... Components, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::ViewComponent(ExcludeType<ExcludeFilter...>) -> decltype(auto)
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

        return m_Registry.view<Components...>(Exclude<ExcludeFilter...>);
    }

    template <typename... Components, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::ViewComponent(ExcludeType<ExcludeFilter...>) const -> decltype(auto)
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

        return m_Registry.view<Components...>(Exclude<ExcludeFilter...>);
    }

    template <typename... Owned, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::GroupComponent(ExcludeType<ExcludeFilter...>) -> decltype(auto)
    {
        static_assert(sizeof...(Owned) > 0, "Exclusion-only groups are not supported");
        static_assert(sizeof...(Owned) + sizeof...(ExcludeFilter) > 1, "Single component groups are not allowed");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Owned>, Owned> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

        return m_Registry.group<Owned...>(Exclude<ExcludeFilter...>);
    }

    template <typename... Owned, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::GroupComponent(ExcludeType<ExcludeFilter...>) const -> decltype(auto)
    {
        static_assert(sizeof...(Owned) > 0, "Exclusion-only groups are not supported");
        static_assert(sizeof...(Owned) + sizeof...(ExcludeFilter) > 1, "Single component groups are not allowed");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Owned>, Owned> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

        return m_Registry.group<Owned...>(Exclude<ExcludeFilter...>);
    }

    template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...>) -> decltype(auto)
    {
        static_assert(sizeof...(Owned) > 0, "Exclusion-only groups are not supported");
        static_assert(sizeof...(Owned) + sizeof...(GetFilter) + sizeof...(ExcludeFilter) > 1, "Single component groups are not allowed");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Owned>, Owned> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<GetFilter>, GetFilter> && ...), "Get type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

        return m_Registry.group<Owned...>(Get<GetFilter...>, Exclude<ExcludeFilter...>);
    }

    template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...>) const -> decltype(auto) {
        static_assert(sizeof...(Owned) + sizeof...(GetFilter) > 0, "Exclusion-only groups are not supported");
        static_assert(sizeof...(Owned) + sizeof...(GetFilter) + sizeof...(ExcludeFilter) > 1, "Single component groups are not allowed");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Owned>, Owned> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<GetFilter>, GetFilter> && ...), "Get type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

        return m_Registry.group<Owned...>(Get<GetFilter...>, Exclude<ExcludeFilter...>);
    }

    template <typename... Components>
    auto EntityComponent::RemoveComponent(const Entity id) -> void
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

        HELENA_ASSERT(HasEntity(id), "Entity {} is not valid", id);

        if constexpr (sizeof...(Components) == 1) {
            HELENA_ASSERT(HasComponent<Components...>(id), "Entity {}, component {} is not exist", id, Traits::NameOf<Components...>::value);
            if(HasComponent<Components...>(id)) {
                Engine::SignalEvent<Events::EntityComponent::RemoveComponent<Components...>>(id);
                m_Registry.remove<Components...>(id);
            }
        } else {
            (RemoveComponent<Components>(id), ...);
        }
    }

    template <typename... Components, typename It>
    auto EntityComponent::RemoveComponent(It first, It last) -> void {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

        for(; first != last; ++first) {
            RemoveComponent<Components...>(*first);
        }
    }

    template <typename... Components>
    auto EntityComponent::ClearComponent() -> void {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...),
                "Component type cannot be const/ptr/ref");

        if constexpr (sizeof...(Components) == 1) {
            const auto view = m_Registry.view<Components...>();
            RemoveComponent<Components...>(view.begin(), view.end());
        } else {
            (ClearComponent<Components>, ...);
        }
    }

    inline auto EntityComponent::Clear() -> void
    {
        m_Registry.each([this](const auto id) {
            RemoveEntity(id);
        });
    }

    template <typename Component>
    auto EntityComponent::SizeComponent() const -> std::size_t {
        static_assert(std::is_same_v<Traits::RemoveCVRefPtr<Component>, Component>,
            "Component type cannot be const/ptr/ref");

        return m_Registry.size<Component>();
    }

    template <typename... Components>
    auto EntityComponent::ReserveComponent(const std::size_t size) -> void {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Traits::RemoveCVRefPtr<Components>, Components> && ...),
            "Component type cannot be const/ptr/ref");

        m_Registry.reserve<Components...>(size);
    }
}

#endif // HELENA_SYSTEMS_ENTITYCOMPONENT_IPP
