#ifndef COMMON_SYSTEMS_ENTITYCOMPONENT_IPP
#define COMMON_SYSTEMS_ENTITYCOMPONENT_IPP

#include <Common/Helena.hpp>

namespace Helena::Systems
{
    inline EntityComponent::~EntityComponent() {
        Clear();
    }

    [[nodiscard]] inline auto EntityComponent::CreateEntity() -> Entity
    {
        using Event = Events::Systems::EntityComponent::CreateEntity;
        const auto entity = m_Registry.create();

        Core::TriggerEvent<Event>(entity);

        return entity;
    }

    template <typename Type, typename>
    [[nodiscard]] auto EntityComponent::CreateEntity(const Type id) -> Entity
    {
        using Event = Events::Systems::EntityComponent::CreateEntity;

        HF_ASSERT(!HasEntity(static_cast<const Entity>(id)), "Entity {} is already exist", id);
        const auto entity = m_Registry.create(static_cast<const entt::entity>(id));
        Core::TriggerEvent<Event>(entity);

        return entity;
    }

    template<typename It>
    auto EntityComponent::CreateEntity(It first, It last) -> void {
        using Event = Events::Systems::EntityComponent::CreateEntity;

        m_Registry.create(first, last);
        for(; first != last; ++first) {
            Core::TriggerEvent<Event>(*first);
        }
    }

    [[nodiscard]] inline auto EntityComponent::HasEntity(const Entity id) const -> bool {
        return m_Registry.valid(id);
    }

    [[nodiscard]] inline auto EntityComponent::SizeEntity() const noexcept -> std::size_t {
        return m_Registry.size();
    }

    [[nodiscard]] inline auto EntityComponent::AliveEntity() const noexcept -> std::size_t {
        return m_Registry.alive();
    }

    inline auto EntityComponent::ReserveEntity(const std::size_t size) -> void {
        m_Registry.reserve(size);
    }

    inline auto EntityComponent::RemoveEntity(const Entity id) -> void
    {
        using Event = Events::Systems::EntityComponent::RemoveEntity;

        HF_ASSERT(HasEntity(id), "Entity {} is not valid", id);
        Core::TriggerEvent<Event>(id);
        m_Registry.destroy(id);
    }

    template<typename It>
    auto EntityComponent::RemoveEntity(It first, It last) -> void
    {
        using Event = Events::Systems::EntityComponent::RemoveEntity;

        for(; first != last; ++first) {
            HF_ASSERT(HasEntity(*first), "Entity {} is not valid", *first);
            Core::TriggerEvent<Event>(*first);
            m_Registry.destroy(*first);
        }
    }

    template <typename Type, typename>
    [[nodiscard]] auto EntityComponent::Cast(const Type id) noexcept {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Component type cannot be const/ptr/ref");
        return static_cast<std::underlying_type_t<Entity>>(id);
    }

    [[nodiscard]] auto EntityComponent::Cast(const Entity id) noexcept {
        return entt::to_integral(id);
    }

    template <typename Func>
    auto EntityComponent::Each(Func&& callback) const -> void {
        static_assert(std::is_function_v<Func>, "Func is not a callable type");
        m_Registry.each(std::forward<Func>(callback));
    }

    template <typename Func>
    auto EntityComponent::EachOrphans(Func&& callback) const -> void {
        static_assert(std::is_function_v<Func>, "Func is not a callable type");
        m_Registry.orphans(std::forward<Func>(callback));
    }

    // Components
    template <typename Component, typename... Args>
    auto EntityComponent::AddComponent(const Entity id, Args&&... args) -> void
    {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Component>, Component>, "Component type cannot be const/ptr/ref");
        using Event = Events::Systems::EntityComponent::AddComponent<Component>;

        HF_ASSERT(HasEntity(id), "Entity {} is not valid", id);
        HF_ASSERT(!HasComponent<Component>(id), "Entity {}, component {} is already exist", id, Internal::type_name_t<Component>);

        m_Registry.emplace<Component>(id, std::forward<Args>(args)...);
        Core::TriggerEvent<Event>(id);
    }

    //template <typename Component>
    //auto EntityComponent::AddComponentTag(const Entity id) -> void {
    //	static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Component>, Component>, "Component type cannot be const/ptr/ref");

    //	HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
    //	HF_ASSERT(!HasComponent<Component>(id), "EntityID: {}, component: {} already exist", id, Internal::type_name_t<Component>);

    //	m_Registry.emplace<Component>(id);
    //	//Core::TriggerEvent<Events::Systems::EntityComponent::AddComponent<Component>>(id, m_Registry.get<Component>(id));
    //}

    //template <typename Component, typename... Args>
    //auto EntityComponent::AddOrGetComponent(const Entity id, Args&&... args) -> Component& {
    //	static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Component>, Component>, "Component type cannot be const/ptr/ref");
    //	static_assert(!Internal::is_integral_constant_v<Component>, "This method not support tags!");

    //	HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);

    //	if(HasComponent<Component>(id)) {
    //		return m_Registry.get<Components...>(id);
    //	}

    //	auto& component = m_Registry.emplace<Component>(id, std::forward<Args>(args)...);
    //	Core::TriggerEvent<Events::Systems::EntityComponent::AddComponent<Component>>(id, component);
    //	return component;
    //}

    //template <typename Component, typename... Args>
    //auto EntityComponent::AddOrReplaceComponent(const Entity id, Args&&... args) -> Component& {
    //	static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Component>, Component>, "Component type cannot be const/ptr/ref");

    //	HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);

    //	if(HasComponent<Component>(id)) {
    //		return m_Registry.replace<Components...>(id);
    //	}

    //	auto& component = m_Registry.emplace<Component>(id, std::forward<Args>(args)...);
    //	Core::TriggerEvent<Events::Systems::EntityComponent::AddComponent<Component>>(id, component);
    //	return component;
    //}

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::GetComponent(const Entity id) -> decltype(auto)
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert(((!Internal::is_integral_constant_v<Components>) && ...), "This method not support tags!");

        HF_ASSERT(HasComponent<Components...>(id), "Entity id {} one of the components is not exist!", id);
        return m_Registry.get<Components...>(id);
    }

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::GetComponent(const Entity id) const -> decltype(auto)
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert(((!Internal::is_integral_constant_v<Components>) && ...), "This method not support tags!");

        HF_ASSERT(HasEntity(id), "Entity {} not valid", id);
        HF_ASSERT(HasComponent<Components...>(id), "Entity {} one of the components is not exist!", id);

        return m_Registry.get<Components...>(id);
    }

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::GetComponentPtr(const Entity id)
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert(((!Internal::is_integral_constant_v<Components>) && ...), "This method not support tags!");

        HF_ASSERT(HasEntity(id), "Entity {} not valid", id);
        return m_Registry.try_get<Components...>(id);
    }

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::GetComponentPtr(const Entity id) const
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert(((!Internal::is_integral_constant_v<Components>) && ...), "This method not support tags!");

        HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
        return m_Registry.try_get<Components...>(id);
    }

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::HasComponent(const Entity id) const -> bool
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

        HF_ASSERT(HasEntity(id), "Entity {} not valid", id);
        return m_Registry.all_of<Components...>(id);
    }

    [[nodiscard]] inline auto EntityComponent::HasComponent(const Entity id) const -> bool {
        return !m_Registry.orphan(id);
    }

    template <typename... Components>
    [[nodiscard]] auto EntityComponent::AnyComponent(const Entity id) const -> bool
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

        return m_Registry.any_of<Components...>(id);
    }

    template <typename Func>
    auto EntityComponent::VisitComponent(const Entity id, Func&& callback) const -> void
    {
        static_assert(std::is_function_v<Func>, "Func is not a callable type");

        HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
        m_Registry.visit(id, std::forward<Func>(callback));
    }

    template <typename Func>
    auto EntityComponent::VisitComponent(Func&& callback) const -> void {
        static_assert(std::is_function_v<Func>, "Func is not a callable type");
        m_Registry.visit(std::forward<Func>(callback));
    }

    template <typename... Components, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::ViewComponent(ExcludeType<ExcludeFilter...>) -> decltype(auto)
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");


        return m_Registry.view<Components...>(Exclude<ExcludeFilter...>);
    }

    template <typename... Components, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::ViewComponent(ExcludeType<ExcludeFilter...>) const -> decltype(auto)
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

        return m_Registry.view<Components...>(Exclude<ExcludeFilter...>);
    }

    template <typename... Owned, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::GroupComponent(ExcludeType<ExcludeFilter...>) -> decltype(auto)
    {
        static_assert(sizeof...(Owned) > 0, "Exclusion-only groups are not supported");
        static_assert(sizeof...(Owned) + sizeof...(ExcludeFilter) > 1, "Single component groups are not allowed");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Owned>, Owned> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

        return m_Registry.group<Owned...>(Exclude<ExcludeFilter...>);
    }

    template <typename... Owned, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::GroupComponent(ExcludeType<ExcludeFilter...>) const -> decltype(auto)
    {
        static_assert(sizeof...(Owned) > 0, "Exclusion-only groups are not supported");
        static_assert(sizeof...(Owned) + sizeof...(ExcludeFilter) > 1, "Single component groups are not allowed");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Owned>, Owned> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

        return m_Registry.group<Owned...>(Exclude<ExcludeFilter...>);
    }

    template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...>) -> decltype(auto)
    {
        static_assert(sizeof...(Owned) > 0, "Exclusion-only groups are not supported");
        static_assert(sizeof...(Owned) + sizeof...(GetFilter) + sizeof...(ExcludeFilter) > 1, "Single component groups are not allowed");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Owned>, Owned> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<GetFilter>, GetFilter> && ...), "Get type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

        return m_Registry.group<Owned...>(Get<GetFilter...>, Exclude<ExcludeFilter...>);
    }

    template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
    [[nodiscard]] auto EntityComponent::GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...>) const -> decltype(auto) {
        static_assert(sizeof...(Owned) + sizeof...(GetFilter) > 0, "Exclusion-only groups are not supported");
        static_assert(sizeof...(Owned) + sizeof...(GetFilter) + sizeof...(ExcludeFilter) > 1, "Single component groups are not allowed");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Owned>, Owned> && ...), "Component type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<GetFilter>, GetFilter> && ...), "Get type cannot be const/ptr/ref");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

        return m_Registry.group<Owned...>(Get<GetFilter...>, Exclude<ExcludeFilter...>);
    }

    template <typename... Components>
    auto EntityComponent::RemoveComponent(const Entity id) -> void
    {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

        HF_ASSERT(HasEntity(id), "Entity {} is not valid", id);

        if constexpr (sizeof...(Components) == 1) {
            using Event = Events::Systems::EntityComponent::RemoveComponent<Components...>;

            HF_ASSERT(HasComponent<Components...>(id), "Entity {}, component {} is not exist", id, Internal::type_name_t<Components...>);
            if(HasComponent<Components...>(id)) {
                Core::TriggerEvent<Event>(id);
                m_Registry.remove<Components...>(id);
            }
        } else {
            (RemoveComponent<Components>(id), ...);
        }
    }

    template <typename... Components, typename It>
    auto EntityComponent::RemoveComponent(It first, It last) -> void {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

        for(; first != last; ++first) {
            RemoveComponent<Components...>(*first);
        }
    }

    inline auto EntityComponent::RemoveComponents(const Entity id) -> void
    {
        using Event = Events::Systems::EntityComponent::RemoveComponents;

        HF_ASSERT(HasEntity(id), "Entity {} is not valid", id);
        Core::TriggerEvent<Event>(id);
        m_Registry.remove_all(id);
    }

    template <typename... Components>
    auto EntityComponent::ClearComponent() -> void {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...),
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
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Component>, Component>,
            "Component type cannot be const/ptr/ref");

        return m_Registry.size<Component>();
    }

    template <typename... Components>
    auto EntityComponent::ReserveComponent(const std::size_t size) -> void {
        static_assert(sizeof...(Components) > 0, "Components pack is empty");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...),
            "Component type cannot be const/ptr/ref");

        m_Registry.reserve<Components...>(size);
    }

    inline auto EntityComponent::ReservePool(const std::size_t size) -> void {
        m_Registry.reserve_pools(size);
    }
}

#endif // COMMON_SYSTEMS_ENTITYCOMPONENT_IPP
