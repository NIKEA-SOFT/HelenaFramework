#ifndef COMMON_SYSTEMS_CONFIGMANAGER_IPP
#define COMMON_SYSTEMS_CONFIGMANAGER_IPP

namespace Helena::Systems
{
	template <typename Resource>
	[[nodiscard]] auto ConfigManager::ResourceIndex<Resource>::GetIndex(map_index_t& container) -> std::size_t {
		static const std::size_t value = Util::AddOrGetTypeIndex(container, Hash::Type<Resource>);
		return value;
	}

	template <typename Resource, typename Propertie>
	[[nodiscard]] auto ConfigManager::PropertieIndex<Resource, Propertie>::GetIndex(map_index_t& container) -> std::size_t {
		static const std::size_t value = Util::AddOrGetTypeIndex(container, Hash::Type<Propertie>);
		return value;
	}

	//template <typename Resource>
	//template <typename Propertie, typename... Args>
	//auto ConfigManager::Storage<Resource>::AddPropertie<Propertie>(const ResourceID hash, Args&&... args) -> void {
	//	static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Propertie>, Propertie>, "Propertie type cannot be const/ptr/ref");

	//	const auto index = PropertieIndex<Resource, Propertie>::GetIndex(m_PropertieIndexes);
	//	if(index >= m_Properties.size()) {
	//		m_Properties.resize(index + 1);
	//	}

	//	m_Properties[index].emplace(std::forward<Args>(args)...);

	//}


	template <typename Resource, typename... Args>
	auto ConfigManager::AddResource([[maybe_unused]] Args&&... args) -> void 
	{
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");

		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
		if(index >= m_Resources.size()) {
			m_Resources.resize(index + 1);
		}

		HF_ASSERT(!m_Resources[index], "Resource type: {} already exist", Internal::type_name_t<Resource>);
		m_Resources[index].template emplace<Resource>(std::forward<Args>(args)...);
	}

	template <typename Resource>
	auto ConfigManager::HasResource() noexcept -> bool {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");

		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
		return index < m_Resources.size() && m_Resources[index];
	}

	template <typename Resource>
	auto ConfigManager::GetResource() noexcept -> Resource& 
	{
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");

		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
		HF_ASSERT(index < m_Resources.size() && m_Resources[index], "Instance of Resource {} does not exist", Internal::type_name_t<Resource>);
		return entt::any_cast<Resource&>(m_Resources[index]);
	}

	template <typename Resource>
	auto ConfigManager::RemoveResource() noexcept -> void {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");

		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
		HF_ASSERT(index < m_Resources.size() && m_Resources[index], "Instance of Resource {} does not exist for remove", Internal::type_name_t<Resource>);
		m_Resources[index].reset();
	}
}

#endif // COMMON_SYSTEMS_CONFIGMANAGER_IPP