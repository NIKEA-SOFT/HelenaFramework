#pragma once

#ifndef COMMON_RESOURCEMANAGER_IPP
#define COMMON_RESOURCEMANAGER_IPP

namespace Helena
{
	inline auto ResourceManager::GetContext() -> ResourceManagerCtx* {
		return Core::CreateCtx<ResourceManagerCtx>();
	}

	inline auto ResourceManager::GetStorageIndex(entt::id_type key) noexcept -> entt::entity {
		const auto ctx = GetContext();
		auto& registry = ctx->m_Registry;
		auto& indexes = ctx->m_StorageIndexes;
		const auto it = indexes.find(key);

		return it != indexes.cend() ? it->second : indexes.emplace(key, registry.create()).first->second;
	}

	template <typename Type>
	template <typename... Args>
	auto ResourceManager::Storage<Type>::CreateResource(Args&&... args) -> Resource<Type>& {
		constexpr auto key = entt::type_hash<Type>::value();
		//HF_ASSERT(m_Resources.find(key) == m_Resources.cend(), "Resource<{}> already exist!", entt::type_name<Type>::value());
		const auto [it, result] = m_Resources.emplace(key, HF_NEW Type{std::forward<Args>(args)...});
		//HF_ASSERT(result, "Resource<{}> allocate memory failed!",  entt::type_name<Type>::value());
		return it->second;
	}

	template <typename Type>
	template <typename... Args>
	auto ResourceManager::Storage<Type>::AddResource(entt::id_type key, Args&&... args) -> Resource<Type>& {
		//HF_ASSERT(m_Resources.find(key) == m_Resources.cend(), "Resource<{}> already exist!", entt::type_name<Type>::value());
		const auto [it, result] = m_Resources.emplace(key, HF_NEW Type{std::forward<Args>(args)...});
		//HF_ASSERT(result, "Resource<{}> allocate memory failed!",  entt::type_name<Type>::value());
		return it->second;
	}

	template <typename Type>
	auto ResourceManager::Storage<Type>::GetResource(entt::id_type key) noexcept -> Resource<Type>& {
		const auto it = m_Resources.find(key);
		//HF_ASSERT(it != m_Resources.cend(), "Resource key: {} not exist!", key);
		return it != m_Resources.cend() ? it->second : ResourceNull<Resource<Type>>::Get();
	}

	template <typename Type>
	auto ResourceManager::Storage<Type>::GetResources() noexcept -> const ResourceMap& {
		return m_Resources;
	}

	template <typename Type>
	auto ResourceManager::Storage<Type>::RemoveResource(entt::id_type key) -> void {
		m_Resources.erase(key);
	}
	
	template <typename Type>
	template <typename Callback>
	auto ResourceManager::Storage<Type>::EachResources(Callback func) -> void {
		static_assert(std::is_invocable_v<Callback, const entt::id_type, const Resource<Type>&>, "Callback is not invocable");
		static_assert(std::is_same_v<void, std::invoke_result_t<Callback, const entt::id_type, const Resource<Type>&>>, "Callback args incorrect");

		for(const auto& pair : m_Resources) {
			func(pair.first, pair.second);
		}
	}

	template <typename Type>
	auto ResourceManager::SetStorage(const Container<Type>& storage) -> void {
		auto& registry = GetContext()->m_Registry;
		const auto index = StorageIndexer<Type>::GetIndex();
		if(!registry.has<Container<Type>>(index)) {
			registry.emplace<Container<Type>>(index, storage);
		} else {
			registry.get<Container<Type>>(index).reset(storage);
		}
	}

	template <typename Type>
	auto ResourceManager::CreateStorage() -> Container<Type>& {
		auto& registry = GetContext()->m_Registry;
		const auto index = StorageIndexer<Type>::GetIndex();
		//HF_ASSERT(!registry.has<Container<Type>>(index), "Storage<{}> already exist!", entt::type_name<Type>::value());
		return registry.emplace<Container<Type>>(index, std::make_shared<Storage<Type>>());
	}

	template <typename Type>
	auto ResourceManager::ExtractStorage() noexcept -> Container<Type> {
		auto& registry = GetContext()->m_Registry;
		const auto index = StorageIndexer<Type>::GetIndex();
		//HF_ASSERT(registry.has<Container<Type>>(index), "Storage<{}> not exist!", entt::type_name<Type>::value());
		auto storage = std::move(registry.get<Container<Type>>(index));
		registry.remove<Container<Type>>(index);
		return storage;
	}

	template <typename Type>
	auto ResourceManager::HasStorage() noexcept -> bool {
		auto& registry = GetContext()->m_Registry;
		const auto index = StorageIndexer<Type>::GetIndex();
		return registry.has<Container<Type>>(index) ? true : false;
	}

	template <typename Type>
	auto ResourceManager::GetStorage() noexcept -> Container<Type>& {
		const auto& registry = GetContext().m_Registry;
		const auto index = StorageIndexer<Type>::GetIndex();
		//HF_ASSERT(registry.has<Container<Type>>(index), "Storage<{}> not exist!", entt::type_name<Type>::value());
		return registry.has<Container<Type>>(index) ? registry.get<Container<Type>>(index) : ResourceNull<Container<Type>>::Get();
	}

	template <typename Type>
	auto ResourceManager::RemoveStorage() -> void {
		auto& registry = GetContext()->m_Registry;
		const auto index = StorageIndexer<Type>::GetIndex();
		//HF_ASSERT(registry.has<Container<Type>>(index), "Storage<{}> not exist!", entt::type_name<Type>::value());
		if(registry.has<Container<Type>>(index)) {
			registry.remove<Container<Type>>(index);
		}
	}
}

#endif // COMMON_RESOURCEMANAGER_IPP