#ifndef COMMON_ECSYSTEM_IPP
#define COMMON_ECSYSTEM_IPP

namespace Helena::Systems
{
	inline auto ECSystem::CreateEntity() -> Entity {
		return m_Registry.create();
	}

	inline auto ECSystem::CreateEntity(const Entity id) -> Entity {
		return m_Registry.create(id);
	}

	template<typename It>
	auto ECSystem::CreateEntity(It first, It last) -> void {
		m_Registry.create(first, last);
	}

	inline auto ECSystem::HasEntity(const Entity id) const -> bool {
		return m_Registry.valid(id);
	}

	inline auto ECSystem::SizeEntity() const -> std::size_t {
		return m_Registry.size();
	}

	inline auto ECSystem::RemoveEntity(const Entity id) -> void {
		//HF_ASSERT(m_Registry.valid(id), "Entity id: {} not valid", id);
		m_Registry.destroy(id);
	}

	template<typename It>
	auto ECSystem::RemoveEntity(It first, It last) -> void {
		m_Registry.destroy(first, last);
	}

	template <typename Func>
	auto ECSystem::Each(Func&& callback) const -> void {
		m_Registry.each(std::move(callback));
	}

	// Components
	template <typename Component, typename... Args>
	auto ECSystem::AddComponent(const Entity id, Args&&... args) -> Component& {
		//HF_ASSERT(m_Registry.valid(id), "Entity id: {} not valid", id);
		//HF_ASSERT(!m_Registry.has<Component>(id), "Entity id: {} component: {} already has!", id, Internal::type_name_t<Component>);
		return m_Registry.emplace<Component>(id, std::forward<Args>(args)...);
	}

	template <typename Component, typename... Args>
	auto ECSystem::AddOrGetComponent(const Entity id, Args&&... args) -> Component& {
		//HF_ASSERT(m_Registry.valid(id), "Entity id: {} not valid", id);
		return m_Registry.get_or_emplace<Component>(id, std::forward<Args>(args)...);
	}

	template <typename Component, typename... Args>
	auto ECSystem::AddOrReplaceComponent(const Entity id, Args&&... args) -> Component& {
		//HF_ASSERT(m_Registry.valid(id), "Entity id: {} not valid", id);
		return m_Registry.emplace_or_replace<Component>(id, std::forward<Args>(args)...);
	}

	template <typename... Components>
	[[nodiscard]] auto ECSystem::GetComponent(const Entity id) -> decltype(auto) {
		//HF_ASSERT(m_Registry.valid(id), "Entity id: {} not valid", id);
		//HF_ASSERT(m_Registry.has<Components...>(id), "Entity id: {} one of the components is not exist!", id);
		return m_Registry.get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto ECSystem::GetComponent(const Entity id) const -> decltype(auto) {
		//HF_ASSERT(m_Registry.valid(id), "Entity id: {} not valid", id);
		//HF_ASSERT(m_Registry.has<Components...>(id), "Entity id: {} one of the components is not exist!", id);
		return m_Registry.get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto ECSystem::GetComponentPtr(const Entity id) {
		//HF_ASSERT(m_Registry.valid(id), "Entity id: {} not valid", id);
		return m_Registry.try_get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto ECSystem::GetComponentPtr(const Entity id) const {
		//HF_ASSERT(m_Registry.valid(id), "Entity id: {} not valid", id);
		return m_Registry.try_get<Components...>(id);
	}

	template <typename... Components>
	[[nodiscard]] auto ECSystem::HasComponent(const Entity id) -> bool {
		//HF_ASSERT(m_Registry.valid(id), "Entity id: {} not valid", id);
		return m_Registry.has<Components...>();
	}

	template <typename... Components, typename... ExcludeFilter>
	[[nodiscard]] auto ECSystem::ViewComponent(ExcludeType<ExcludeFilter...>) -> decltype(auto) {
		return m_Registry.view<Components...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Components, typename... ExcludeFilter>
	[[nodiscard]] auto ECSystem::ViewComponent(ExcludeType<ExcludeFilter...>) const -> decltype(auto) {
		return m_Registry.view<Components...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Owned, typename... ExcludeFilter>
	[[nodiscard]] auto ECSystem::GroupComponent(ExcludeType<ExcludeFilter...>) -> decltype(auto) {
		return m_Registry.group<Owned...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Owned, typename... ExcludeFilter>
	[[nodiscard]] auto ECSystem::GroupComponent(ExcludeType<ExcludeFilter...>) const -> decltype(auto) {
		return m_Registry.group<Owned...>(Exclude<ExcludeFilter...>);
	}

	template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
	[[nodiscard]] auto ECSystem::GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...>) -> decltype(auto) {
		return m_Registry.group<Owned...>(Get<GetFilter...>, Exclude<ExcludeFilter...>);
	}

	template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
	[[nodiscard]] auto ECSystem::GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...>) const -> decltype(auto) {
		return m_Registry.group<Owned...>(Get<GetFilter...>, Exclude<ExcludeFilter...>);
	}

	template <typename... Components>
	auto ECSystem::RemoveComponent(const Entity id) -> void {
		//HF_ASSERT(m_Registry.valid(id), "Entity id: {} not valid", id);
		m_Registry.remove_if_exists<Components...>(id);
	}

	template <typename... Components, typename It>
	auto ECSystem::RemoveComponent(It first, It last) -> void {
		m_Registry.view<Components...>();
		m_Registry.remove<Components...>(first, last);
	}

	inline auto ECSystem::RemoveComponents(const Entity id) -> void {
		m_Registry.remove_all(id);
	}

	template <typename... Components>
	auto ECSystem::ClearComponent() -> void {
		m_Registry.clear<Components...>();
	}

	template <typename Component>
	auto ECSystem::SizeComponent() const -> std::size_t {
		return m_Registry.size<Component>();
	}

	template <typename... Components>
	auto ECSystem::AnyComponent(const Entity id) const -> bool {
		return m_Registry.any<Components...>(id);
	}
}

#endif // COMMON_ECSYSTEM_IPP