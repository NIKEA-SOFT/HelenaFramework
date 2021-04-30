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

	template <typename Resource>
	template <typename Propertie, typename... Args>
	auto ConfigManager::Storage<Resource>::AddPropertie<Propertie>(const ResourceID hash, Args&&... args) -> void {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Propertie>, Propertie>, "Propertie type cannot be const/ptr/ref");

		const auto index = PropertieIndex<Resource, Propertie>::GetIndex(m_PropertieIndexes);
		if(index >= m_Properties.size()) {
			m_Properties.resize(index + 1);
		}

		m_Properties[index].emplace(std::forward<Args>(args)...);

	}


	template <typename Resource, typename... Args>
	auto ConfigManager::AddResource(Args&&... args) -> void {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");

		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
		if(index >= m_Storage.size()) {
			m_Storage.resize(index + 1);
		}

		HF_ASSERT(!m_Storage[index].m_Instance, "Resource type: {} already exist", Internal::type_name_t<Resource>);
		m_Storage[index].m_Instance.emplace<Storage<Resource>>(std::forward<Args>(args)...);
	}

	template <typename Resource>
	auto ConfigManager::GetResource() -> Resource& {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");

		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
		if(index < m_Storage.size() && ) {

		}
		return index < m_Storage.size() ? entt::any_cast<Storage<Resource>&>(m_Storage[index]).m_Instance: nullptr;
	}

	template <typename Resource>
	auto ConfigManager::RemoveResource() -> void {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");

		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
		HF_ASSERT(m_Storage[index].m_Instance, "Resource: {} not exist for remove", Internal::type_name_t<Resource>);
		if(index < m_Storage.size()) {
			m_Storage[index].m_Instance.reset();
		}
	}
}

#endif // COMMON_SYSTEMS_CONFIGMANAGER_IPP