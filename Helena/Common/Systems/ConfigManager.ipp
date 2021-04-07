#ifndef COMMON_SYSTEMS_CONFIGMANAGER_IPP
#define COMMON_SYSTEMS_CONFIGMANAGER_IPP

namespace Helena::Systems
{
	template <typename Resource>
	[[nodiscard]] auto ConfigManager::ResourceIndex<Resource>::GetIndex() -> std::size_t {
		static const auto value = 0u; //Util::AddOrGetTypeIndex(m_Indexes, Internal::type_hash_t<Resource>);
		return value;
	}

	template <typename Resource, typename... Args>
	auto ConfigManager::AddResource(Args&&... args) -> Resource& {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, 
			"Resource type cannot be const/ptr/ref");
		static_assert(std::is_constructible_v<Resource, Args...>, 
			"Resource type cannot be constructable!");

		const auto index = ResourceIndex<Resource>::GetIndex();
		if(index >= m_Storage.size()) {
			m_Storage.resize(index + 1);
		}

		auto& resource = m_Storage[index];
		if(!resource) {
			resource.template emplace<Resource>(std::forward<Args>(args)...);
		}

		return entt::any_cast<Resource&>(resource);
	}

}

#endif // COMMON_SYSTEMS_CONFIGMANAGER_IPP