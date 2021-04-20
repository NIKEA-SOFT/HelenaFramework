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
	template <ConfigManager::KeyID Hash, typename Propertie, typename... Args>
	auto ConfigManager::Storage<Resource>::AddPropertie<Hash, Propertie>(Args&&... args) -> decltype(auto) {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Propertie>, Propertie>, "Propertie type cannot be const/ptr/ref");
	}


	template <typename Resource, typename... Args>
	auto ConfigManager::AddResource(Args&&... args) -> Resource* {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");

		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
		if(index >= m_Storage.size()) {
			m_Storage.resize(index + 1);
		}

		entt::hashed_string{
		auto& resource = m_Storage[index];
		resource.emplace<Resource>();
		return entt::any_cast<Resource<Resource, PropertieList...>&>(resource);
	}

	template <typename Resource>
	auto ConfigManager::GetResource() -> Resource* {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");

		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
		return index < m_Storage.size() ? entt::any_cast<Resource>(&m_Storage[index].m_Instance) : nullptr;
	}

	template <typename Resource>
	auto ConfigManager::RemoveResource() -> void {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");

		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
		if(index < m_Storage.size()) {
			m_Storage[index].reset();
		}
	}
}

#endif // COMMON_SYSTEMS_CONFIGMANAGER_IPP