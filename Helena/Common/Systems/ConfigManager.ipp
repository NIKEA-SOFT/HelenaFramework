#ifndef COMMON_SYSTEMS_CONFIGMANAGER_IPP
#define COMMON_SYSTEMS_CONFIGMANAGER_IPP

namespace Helena::Systems
{
	template <typename Resource>
	[[nodiscard]] auto ConfigManager::ResourceIndex<Resource>::GetIndex(map_index_t& container) -> std::size_t {
		static const std::size_t value = Util::AddOrGetTypeIndex(container, Hash::Type<Resource>);
		return value;
	}

    template <typename Resource, typename Property>
    [[nodiscard]] auto ConfigManager::PropertyIndex<Resource, Property>::GetIndex(map_index_t& container) -> std::size_t {
        static const std::size_t value = Util::AddOrGetTypeIndex(container, Hash::Type<Property>);
		return value;
	}

	template <typename Resource, typename... Args>
    auto ConfigManager::CreateResource([[maybe_unused]] Args&&... args) -> void
	{
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>,
                "Resource type cannot be const/ptr/ref");

		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
		if(index >= m_Resources.size()) {
			m_Resources.resize(index + 1);
		}

        HF_ASSERT(!m_Resources[index], "Resource: {} instance already exist", Internal::type_name_t<Resource>);
		m_Resources[index].template emplace<Resource>(std::forward<Args>(args)...);
	}

//	template <typename Resource>
//	auto ConfigManager::HasResource() noexcept -> bool {
//        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>,
//                "Resource type cannot be const/ptr/ref");

//		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
//		return index < m_Resources.size() && m_Resources[index];
//	}

    template <typename... Resources>
    auto ConfigManager::HasResource() noexcept -> bool {
        static_assert(sizeof...(Resources) > 0, "Resource pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Resources>, Resources> && ...),
                "Resource type cannot be const/ptr/ref");

        if constexpr(sizeof...(Resources) == 1) {
            const auto index = ResourceIndex<Resources...>::GetIndex(m_ResourceIndexes);
            return index < m_Resources.size() && m_Resources[index];
        } else {
            return (HasResource<Resources>() && ...);
        }
    }

    template <typename... Resources>
    auto ConfigManager::AnyResource() noexcept -> bool {
        static_assert(sizeof...(Resources) > 0, "Resource pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Resources>, Resources> && ...),
                "Resource type cannot be const/ptr/ref");

        return (HasResource<Resources>() || ...);
    }

//	template <typename Resource>
//	auto ConfigManager::GetResource() noexcept -> Resource&
//	{
//        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>,
//                "Resource type cannot be const/ptr/ref");

//		const auto index = ResourceIndex<Resource>::GetIndex(m_ResourceIndexes);
//        HF_ASSERT(index < m_Resources.size() && m_Resources[index], "Resource {} instance not exist",
//                  Internal::type_name_t<Resource>);
//		return entt::any_cast<Resource&>(m_Resources[index]);
//	}

    template <typename... Resources>
    [[nodiscard]] auto ConfigManager::GetResource() noexcept -> decltype(auto)
    {
        static_assert(sizeof...(Resources) > 0, "Resource pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Resources>, Resources> && ...),
                "Resource type cannot be const/ptr/ref");

        if constexpr(sizeof...(Resources) == 1) {
            const auto index = ResourceIndex<Resources...>::GetIndex(m_ResourceIndexes);
            HF_ASSERT(index < m_Resources.size() && m_Resources[index], "Resource {} instance not exist",
                      Internal::type_name_t<Resources...>);
            return entt::any_cast<Resources&...>(m_Resources[index]);
        } else {
            return std::forward_as_tuple(GetResource<Resources>()...);
        }
    }

    template <typename... Resources>
	auto ConfigManager::RemoveResource() noexcept -> void {
        static_assert(sizeof...(Resources) > 0, "Resource pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Resources>, Resources> && ...),
                "Resource type cannot be const/ptr/ref");

        if constexpr(sizeof...(Resources) == 1) {
            const auto index = ResourceIndex<Resources...>::GetIndex(m_ResourceIndexes);
            HF_ASSERT(index < m_Resources.size() && m_Resources[index], "Resource {} instance not exist for remove",
                      Internal::type_name_t<Resources...>);
            m_Resources[index].reset();
            HF_MSG_DEBUG("Resource: {} removed!", Internal::type_name_t<Resources...>);
        } else {
            (RemoveResource<Resources>(), ...);
        }
	}

    template <typename Resource, typename Key, typename Type, typename... Args>
    auto ConfigManager::AddProperty([[maybe_unused]] Args&&... args) -> void {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");
        static_assert(std::is_class_v<Resource> || Internal::is_integral_constant_v<Resource>, "Resource type can be Class or Resource");
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Key>, Key>, "Key type cannot be const/ptr/ref");
        static_assert(Internal::is_integral_constant_v<Key>, "Key type incorrect, use `ConfigManager::Resource<T>` type");
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Type cannot be const/ptr/ref");

        const auto index = PropertyIndex<Resource, Key>::GetIndex(m_PropertyIndexes);
        if(index >= m_Properties.size()) {
            m_Properties.resize(index + 1);
        }

        HF_ASSERT(!m_Properties[index], "Property {}, key {} instance already exist",
                  Internal::type_name_t<Resource>, Internal::type_name_t<Key>);
        m_Properties[index].template emplace<Type>(std::forward<Args>(args)...);
    }

    template <typename Resource, typename Key>
    auto ConfigManager::HasProperty() noexcept -> bool {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");
        static_assert(std::is_class_v<Resource> || Internal::is_integral_constant_v<Resource>, "Resource type can be Class or Resource");
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Key>, Key>, "Key type cannot be const/ptr/ref");
        static_assert(Internal::is_integral_constant_v<Key>, "Key type incorrect, use `ConfigManager::Resource<T>` type");

        const auto index = PropertyIndex<Resource, Key>::GetIndex(m_PropertyIndexes);
        return index < m_Properties.size() && m_Properties[index];
    }

    template <typename Resource, typename Key, typename Type>
    auto ConfigManager::GetProperty() noexcept -> Type& {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");
        static_assert(std::is_class_v<Resource> || Internal::is_integral_constant_v<Resource>, "Resource type can be Class or Resource");
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Key>, Key>, "Key type cannot be const/ptr/ref");
        static_assert(Internal::is_integral_constant_v<Key>, "Key type incorrect, use `ConfigManager::Resource<T>` type");
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Type cannot be const/ptr/ref");

        const auto index = PropertyIndex<Resource, Key>::GetIndex(m_PropertyIndexes);
        HF_ASSERT(index < m_Properties.size() && m_Properties[index], "Property {}, key {} instance not exist",
                  Internal::type_name_t<Resource>, Internal::type_name_t<Key>);
        HF_ASSERT(entt::any_cast<Type>(&m_Properties[index]), "Incorrect Type {} for property {} with key {}",
                  Internal::type_name_t<Type>, Internal::type_name_t<Resource>, Internal::type_name_t<Key>);
        return entt::any_cast<Type&>(m_Properties[index]);
    }

    template <typename Resource, typename Key>
    auto ConfigManager::RemoveProperty() noexcept -> void {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Resource>, Resource>, "Resource type cannot be const/ptr/ref");
        static_assert(std::is_class_v<Resource> || Internal::is_integral_constant_v<Resource>, "Resource type can be Class or Resource");
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Key>, Key>, "Key type cannot be const/ptr/ref");
        static_assert(Internal::is_integral_constant_v<Key>, "Key type incorrect, use `ConfigManager::Resource<T>` type");

        const auto index = PropertyIndex<Resource, Key>::GetIndex(m_PropertyIndexes);
        HF_ASSERT(index < m_Properties.size() && m_Properties[index], "Property {}, key {} instance not exist",
                  Internal::type_name_t<Resource>, Internal::type_name_t<Key>);
        m_Properties[index].reset();
    }

//    template <typename Type, typename... Args>
//    auto ConfigManager::AddProperty(const std::string_view key, [[maybe_unused]] Args&&... args) -> void {
//        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Type cannot be const/ptr/ref");

//        const auto id = Hash::String(key.data());
//        HF_ASSERT(m_PropertyMap.find(id) == m_PropertyMap.end(), "Property key {} already exist!", key);
//        m_PropertyMap.emplace(id, std::forward<Args>(args)...);
//    }

//    [[nodiscard]] auto ConfigManager::HasProperty(const std::string_view key) const noexcept -> bool {
//        const auto id = Hash::String(key.data());
//        return m_PropertyMap.find(id) != m_PropertyMap.end();
//    }

//    template <typename Type>
//    [[nodiscard]] auto ConfigManager::GetProperty(const std::string_view key) const noexcept -> Type* {
//        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Type cannot be const/ptr/ref");
//        const auto id = Hash::String(key.data());
//        const auto it = m_PropertyMap.find(id);
//        HF_ASSERT(it == m_PropertyMap.end(), "Property key {} not exist!", key);
//        return entt::any_cast<Type>(&*it);
//    }

//    auto ConfigManager::RemoveProperty(const std::string_view key) noexcept -> void {
//        const auto id = Hash::String(key.data());
//        const auto it = m_PropertyMap.find(id);
//        HF_ASSERT(it != m_PropertyMap.end(), "Property key {} already exist!", key);
//        if(it != m_PropertyMap.end()) {
//            m_PropertyMap.erase(id);
//        }
//    }
}

#endif // COMMON_SYSTEMS_CONFIGMANAGER_IPP
