#ifndef COMMON_SYSTEMS_ENTITYCOMPONENT_IPP
#define COMMON_SYSTEMS_ENTITYCOMPONENT_IPP

namespace Helena::Systems
{
	inline EntityComponent::~EntityComponent() {
		Clear();
	}

	inline auto EntityComponent::CreateEntity() -> Entity {
		const auto entity = m_Registry.create();
		Core::TriggerEvent<Events::Systems::EntityComponent::CreateEntity>(entity);
		return entity;
	}

	template <typename Type, typename>
	auto EntityComponent::CreateEntity(const Type id) -> Entity {
		HF_ASSERT(!HasEntity(static_cast<const entt::entity>(id)), "Entity id: {} already exist", id);

		const auto entity = m_Registry.create(static_cast<const entt::entity>(id));
		Core::TriggerEvent<Events::Systems::EntityComponent::CreateEntity>(entity);

		return entity;
	}

	template<typename It>
	auto EntityComponent::CreateEntity(It first, It last) -> void {
		m_Registry.create(first, last);

		for(; first != last; ++first) {
			Core::TriggerEvent<Events::Systems::EntityComponent::CreateEntity>(*first);
		}
	}

	inline auto EntityComponent::HasEntity(const Entity id) const -> bool {
		return m_Registry.valid(id);
	}

	inline auto EntityComponent::SizeEntity() const -> std::size_t {
		return m_Registry.size();
	}

	inline auto EntityComponent::AliveEntity() const -> std::size_t {
		return m_Registry.alive();
	}

	inline auto EntityComponent::ReserveEntity(const std::size_t size) -> void {
		m_Registry.reserve(size);
	}

	inline auto EntityComponent::RemoveEntity(const Entity id) -> void {
		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		Core::TriggerEvent<Events::Systems::EntityComponent::RemoveEntity>(id);
		m_Registry.destroy(id);
	}

	template<typename It>
	auto EntityComponent::RemoveEntity(It first, It last) -> void {
		HF_ASSERT(std::all_of(first, last, [this](const auto id) { 
			return HasEntity(id); 
		}), "One of the Entity is not valid");

		for(; first != last; ++first) {
			Core::TriggerEvent<Events::Systems::EntityComponent::RemoveEntity>(*first);
			m_Registry.destroy(*first);
		}
	}

	inline auto EntityComponent::CastFromEntity(const Entity id) {
		return entt::to_integral(id);
	}

	template <typename Type>
	auto EntityComponent::CastToEntity(const Type id) -> Entity {
		return static_cast<entt::entity>(id);
	}

	template <typename Func>
	auto EntityComponent::Each(Func&& callback) const -> void {
		m_Registry.each(std::forward<Func>(callback));
	}

	template <typename Func>
	auto EntityComponent::EachOrphans(Func&& callback) const -> void {
		m_Registry.orphans(std::forward<Func>(callback));
	}

	// Components
	template <typename Component, typename... Args>
    auto EntityComponent::AddComponent(const Entity id, Args&&... args) -> void {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Component>, Component>, "Component type cannot be const/ptr/ref");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		HF_ASSERT(!HasComponent<Component>(id), "EntityID: {}, component: {} already exist", id, Internal::type_name_t<Component>); 

		m_Registry.emplace<Component>(id, std::forward<Args>(args)...);
        Core::TriggerEvent<Events::Systems::EntityComponent::AddComponent<Component>>(id);
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
	[[nodiscard]] auto EntityComponent::GetComponent(const Entity id) -> decltype(auto) {
		static_assert(sizeof...(Components) > 0, "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
		static_assert(((!Internal::is_integral_constant_v<Components>) && ...), "This method not support tags!");

		HF_ASSERT(HasComponent<Components...>(id), "Entity id: {} one of the components is not exist!", id);

		return m_Registry.get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto EntityComponent::GetComponent(const Entity id) const -> decltype(auto) {
		static_assert(sizeof...(Components) > 0, "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
		static_assert(((!Internal::is_integral_constant_v<Components>) && ...), "This method not support tags!");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		HF_ASSERT(HasComponent<Components...>(id), "Entity id: {} one of the components is not exist!", id);

		return m_Registry.get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto EntityComponent::GetComponentPtr(const Entity id) {
		static_assert(sizeof...(Components) > 0, "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
		static_assert(((!Internal::is_integral_constant_v<Components>) && ...), "This method not support tags!");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		return m_Registry.try_get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto EntityComponent::GetComponentPtr(const Entity id) const {
		static_assert(sizeof...(Components) > 0, "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
		static_assert(((!Internal::is_integral_constant_v<Components>) && ...), "This method not support tags!");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		return m_Registry.try_get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto EntityComponent::HasComponent(const Entity id) -> bool {
		static_assert(sizeof...(Components) > 0, "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
		
		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		return m_Registry.all_of<Components...>(id);
	}

	[[nodiscard]] inline auto EntityComponent::HasComponents(const Entity id) -> bool {
		return !m_Registry.orphan(id);
	}

	template <typename... Components>
	[[nodiscard]] auto EntityComponent::AnyComponent(const Entity id) const -> bool {
		static_assert(sizeof...(Components) > 0, "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

		return m_Registry.any_of<Components...>(id);
	}

	template <typename Func>
	auto EntityComponent::VisitComponent(const Entity id, Func&& callback) const -> void {
		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		m_Registry.visit(id, std::forward<Func>(callback));
	}

	template <typename Func>
	auto EntityComponent::VisitComponent(Func&& callback) const -> void {
		m_Registry.visit(std::forward<Func>(callback));
	}

	template <typename... Components, typename... ExcludeFilter>
	[[nodiscard]] auto EntityComponent::ViewComponent(ExcludeType<ExcludeFilter...>) -> decltype(auto) {
		static_assert(sizeof...(Components) > 0, "Components pack is empty");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");
		

		return m_Registry.view<Components...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Components, typename... ExcludeFilter>
	[[nodiscard]] auto EntityComponent::ViewComponent(ExcludeType<ExcludeFilter...>) const -> decltype(auto) {
		static_assert(sizeof...(Components) > 0, "Components pack is empty");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

		return m_Registry.view<Components...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Owned, typename... ExcludeFilter>
	[[nodiscard]] auto EntityComponent::GroupComponent(ExcludeType<ExcludeFilter...>) -> decltype(auto) {
        static_assert(sizeof...(Owned) > 0, "Exclusion-only groups are not supported");
        static_assert(sizeof...(Owned) + sizeof...(ExcludeFilter) > 1, "Single component groups are not allowed");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Owned>, Owned> && ...), "Component type cannot be const/ptr/ref");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

		return m_Registry.group<Owned...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Owned, typename... ExcludeFilter>
	[[nodiscard]] auto EntityComponent::GroupComponent(ExcludeType<ExcludeFilter...>) const -> decltype(auto) {
        static_assert(sizeof...(Owned) > 0, "Exclusion-only groups are not supported");
        static_assert(sizeof...(Owned) + sizeof...(ExcludeFilter) > 1, "Single component groups are not allowed");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Owned>, Owned> && ...), "Component type cannot be const/ptr/ref");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter> && ...), "Exclude type cannot be const/ptr/ref");

		return m_Registry.group<Owned...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
	[[nodiscard]] auto EntityComponent::GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...>) -> decltype(auto) {
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
	auto EntityComponent::RemoveComponent(const Entity id) -> void {
		static_assert(sizeof...(Components) > 0, "Components pack is empty");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);

		([this, id](const auto identity) {
			using Component = typename decltype(identity)::type;
			if(HasComponent<Component>(id)) {
				Core::TriggerEvent<Events::Systems::EntityComponent::RemoveComponent<Component>>(id);
				m_Registry.remove<Component>(id);
			}
		}(entt::type_identity<Components>{}), ...);
	}

	template <typename... Components, typename It>
	auto EntityComponent::RemoveComponent(It first, It last) -> void {
		static_assert(sizeof...(Components) > 0, "Components pack is empty");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

		for(; first != last; ++first) {
			RemoveComponent<Components...>(*first);
		}
	}

	inline auto EntityComponent::RemoveComponents(const Entity id) -> void {
		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		Core::TriggerEvent<Events::Systems::EntityComponent::RemoveComponents>(id);
		m_Registry.remove_all(id);
	}

	template <typename... Components>
	auto EntityComponent::ClearComponent() -> void {
		static_assert(sizeof...(Components) > 0, "Components pack is empty");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components> && ...), "Component type cannot be const/ptr/ref");

        ([this](const auto identity) {
			using Component = typename Internal::remove_cvrefptr_t<decltype(identity)>::type;
			const auto view = m_Registry.view<Component>();

			for(const auto id : view) {
				Core::TriggerEvent<Events::Systems::EntityComponent::RemoveComponent<Component>>(id);
				m_Registry.remove<Component>(id);
			}
		}(entt::type_identity<Components>{}), ...);
	}

	inline auto EntityComponent::Clear() -> void 
	{
		m_Registry.each([this](const auto id) {
			RemoveEntity(id);
		});
	}

	template <typename Component>
	auto EntityComponent::SizeComponent() -> std::size_t {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Component>, Component>, 
			"Component type cannot be const/ptr/ref");

		return m_Registry.size<Component>();
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
