#ifndef COMMON_SYSTEMS_ENTITYCOMPONENT_IPP
#define COMMON_SYSTEMS_ENTITYCOMPONENT_IPP

namespace Helena::Systems
{
	inline auto EntityComponent::CreateEntity() -> Entity {
		return m_Registry.create();
	}

	template <typename Type, typename>
	auto EntityComponent::CreateEntity(const Type id) -> Entity {
		HF_ASSERT(!HasEntity(static_cast<Entity>(id)), "Entity id: {} already exist", id);
		return m_Registry.create(static_cast<Entity>(id));
	}

	template<typename It>
	auto EntityComponent::CreateEntity(It first, It last) -> void {
		m_Registry.create(first, last);
	}

	inline auto EntityComponent::HasEntity(const Entity id) const -> bool {
		return m_Registry.valid(id);
	}

	inline auto EntityComponent::SizeEntity() const -> std::size_t {
		return m_Registry.size();
	}

	inline auto EntityComponent::ReserveEntity(const std::size_t size) -> void {
		m_Registry.reserve(size);
	}

	inline auto EntityComponent::RemoveEntity(const Entity id) -> void {
		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		m_Registry.destroy(id);
	}

	template<typename It>
	auto EntityComponent::RemoveEntity(It first, It last) -> void {
		HF_ASSERT(std::all_of(first, last, [this](const auto id) { 
			return HasEntity(id); 
		}), "One of the Entity is not valid");

		m_Registry.destroy(first, last);
	}

	inline auto EntityComponent::CastEntity(const Entity id) {
		return static_cast<ENTT_ID_TYPE>(id);
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
	auto EntityComponent::AddComponent(const Entity id, Args&&... args) -> Component& {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Component>, Component>, 
			"Component type cannot be const/ptr/ref");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		HF_ASSERT(!HasComponent<Component>(id), "EntityID: {}, component: {} already exist", 
			id, Internal::type_name_t<Component>); 

		return m_Registry.emplace<Component>(id, std::forward<Args>(args)...);
	}

	template <typename Component, typename... Args>
	auto EntityComponent::AddOrGetComponent(const Entity id, Args&&... args) -> Component& {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Component>, Component>, 
			"Component type cannot be const/ptr/ref");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		return m_Registry.get_or_emplace<Component>(id, std::forward<Args>(args)...);
	}

	template <typename Component, typename... Args>
	auto EntityComponent::AddOrReplaceComponent(const Entity id, Args&&... args) -> Component& {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Component>, Component>, 
			"Component type cannot be const/ptr/ref");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		return m_Registry.emplace_or_replace<Component>(id, std::forward<Args>(args)...);
	}

	template <typename... Components>
	[[nodiscard]] auto EntityComponent::GetComponent(const Entity id) -> decltype(auto) {
		static_assert((sizeof...(Components)), "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...), 
			"Component type cannot be const/ptr/ref");

		HF_ASSERT(HasComponent<Components...>(id), "Entity id: {} one of the components is not exist!", id);
		return m_Registry.get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto EntityComponent::GetComponent(const Entity id) const -> decltype(auto) {
		static_assert((sizeof...(Components)), "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...), 
			"Component type cannot be const/ptr/ref");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		HF_ASSERT(HasComponent<Components...>(id), "Entity id: {} one of the components is not exist!", id);
		return m_Registry.get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto EntityComponent::GetComponentPtr(const Entity id) {
		static_assert((sizeof...(Components)), "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...), 
			"Component type cannot be const/ptr/ref");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		return m_Registry.try_get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto EntityComponent::GetComponentPtr(const Entity id) const {
		static_assert((sizeof...(Components)), "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...), 
			"Component type cannot be const/ptr/ref");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		return m_Registry.try_get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto EntityComponent::HasComponent(const Entity id) -> bool {
		static_assert((sizeof...(Components)), "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...), 
			"Component type cannot be const/ptr/ref");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		return m_Registry.all_of<Components...>(id);
	}

	[[nodiscard]] inline auto EntityComponent::HasComponents(const Entity id) -> bool {
		return !m_Registry.orphan(id);
	}

	template <typename... Components>
	[[nodiscard]] auto EntityComponent::AnyComponent(const Entity id) const -> bool {
		static_assert((sizeof...(Components)), "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...), 
			"Component type cannot be const/ptr/ref");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		return m_Registry.any_of<Components...>(id);
	}

	template <typename Func>
	[[nodiscard]] auto EntityComponent::VisitComponent(const Entity id, Func&& callback) const -> void {
		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		m_Registry.visit(id, std::forward<Func>(callback));
	}

	template <typename Func>
	[[nodiscard]] auto EntityComponent::VisitComponent(Func&& callback) const -> void {
		m_Registry.visit(std::forward<Func>(callback));
	}

	template <typename... Components, typename... ExcludeFilter>
	[[nodiscard]] auto EntityComponent::ViewComponent(ExcludeType<ExcludeFilter...>) -> decltype(auto) {
		static_assert(	(std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...) || 
						(std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter>, ...), 
						"Component and Exclude type cannot be const/ptr/ref");

		return m_Registry.view<Components...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Components, typename... ExcludeFilter>
	[[nodiscard]] auto EntityComponent::ViewComponent(ExcludeType<ExcludeFilter...>) const -> decltype(auto) {
		static_assert(	(std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...) || 
						(std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter>, ...), 
						"Component and Exclude type cannot be const/ptr/ref");

		return m_Registry.view<Components...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Owned, typename... ExcludeFilter>
	[[nodiscard]] auto EntityComponent::GroupComponent(ExcludeType<ExcludeFilter...>) -> decltype(auto) {
		static_assert(	(std::is_same_v<Internal::remove_cvrefptr_t<Owned>, Owned>, ...) || 
						(std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter>, ...), 
						"Owned and Exclude type cannot be const/ptr/ref");

		return m_Registry.group<Owned...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Owned, typename... ExcludeFilter>
	[[nodiscard]] auto EntityComponent::GroupComponent(ExcludeType<ExcludeFilter...>) const -> decltype(auto) {
		static_assert(	(std::is_same_v<Internal::remove_cvrefptr_t<Owned>, Owned>, ...) || 
						(std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter>, ...), 
						"Owned and Exclude type cannot be const/ptr/ref");

		return m_Registry.group<Owned...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
	[[nodiscard]] auto EntityComponent::GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...>) -> decltype(auto) {
		static_assert(	(std::is_same_v<Internal::remove_cvrefptr_t<Owned>, Owned>, ...) || 
						(std::is_same_v<Internal::remove_cvrefptr_t<GetFilter>, GetFilter>, ...) ||
						(std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter>, ...), 
						"Owned, Filter and Exclude type cannot be const/ptr/ref");

		return m_Registry.group<Owned...>(Get<GetFilter...>, Exclude<ExcludeFilter...>);
	}

	template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
	[[nodiscard]] auto EntityComponent::GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...>) const -> decltype(auto) {
		static_assert(	(std::is_same_v<Internal::remove_cvrefptr_t<Owned>, Owned>, ...) || 
						(std::is_same_v<Internal::remove_cvrefptr_t<GetFilter>, GetFilter>, ...) ||
						(std::is_same_v<Internal::remove_cvrefptr_t<ExcludeFilter>, ExcludeFilter>, ...), 
						"Owned, Filter and Exclude type cannot be const/ptr/ref");

		return m_Registry.group<Owned...>(Get<GetFilter...>, Exclude<ExcludeFilter...>);
	}

	template <typename... Components>
	auto EntityComponent::RemoveComponent(const Entity id) -> void {
		static_assert((sizeof...(Components)), "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...), 
			"Component type cannot be const/ptr/ref");

		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		m_Registry.remove_if_exists<Components...>(id);
	}

	template <typename... Components, typename It>
	auto EntityComponent::RemoveComponent(It first, It last) -> void {
		static_assert((sizeof...(Components)), "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...), 
			"Component type cannot be const/ptr/ref");

		HF_ASSERT(std::all_of(first, last, [this](const auto id) { 
			return HasEntity(id); 
		}), "One of the Entity is not valid");

		m_Registry.remove_if_exists<Components...>(first, last);
	}

	inline auto EntityComponent::RemoveComponents(const Entity id) -> void {
		HF_ASSERT(HasEntity(id), "Entity id: {} not valid", id);
		m_Registry.remove_all(id);
	}

	template <typename... Components>
	auto EntityComponent::ClearComponent() -> void {
		static_assert((sizeof...(Components)), "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...), 
			"Component type cannot be const/ptr/ref");

		m_Registry.clear<Components...>();
	}

	inline auto EntityComponent::ClearComponents() -> void {
		m_Registry.clear();
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
		static_assert((sizeof...(Components)), "Components pack is empty!");
		static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Components>, Components>, ...), 
			"Component type cannot be const/ptr/ref");

		m_Registry.reserve<Components...>(size);
	}

	inline auto EntityComponent::ReservePool(const std::size_t size) -> void {
		m_Registry.reserve_pools(size);	
	}
}

#endif // COMMON_SYSTEMS_ENTITYCOMPONENT_IPP