#pragma once

#ifndef COMMON_RESOURCEMANAGER_IPP
#define COMMON_RESOURCEMANAGER_IPP

namespace Helena
{
	inline auto ResourceManager::GetContext() -> ResourceManagerCtx& 
	{
		static auto& ctx{Core::CreateContext<ResourceManagerCtx>()};
		return ctx;
	}

	inline auto ResourceManager::GetResourceEntity(entt::id_type name) noexcept -> entt::entity 
	{
		auto& ctxResource = GetContext();
		auto& idxResource = ctxResource.m_ResourceIndexes;
		auto& regResource = ctxResource.m_Registry;

		const auto it = idxResource.find(name);
		return it == idxResource.cend() ? idxResource.emplace(name, regResource.create()).first->second : it->second;
	}

	template <typename Resource, typename... Args>
	auto ResourceManager::AddResource(entt::id_type id, Args&&... args) -> std::shared_ptr<Resource> {
		auto& storage = HasStorage<Resource>() ? GetStorage<Resource>() : AddStorage<Resource>();
		auto [it, result] = storage.emplace(id, std::make_shared<Resource>(std::forward<Args>(args)...));
		if(!result && it != storage.end()) {
			it->second = std::make_shared<Resource>(std::forward<Args>(args)...);
		}

		return it->second;
	}

	template <typename Callback>
	auto ResourceManager::AddResource(Callback func) -> bool {
		static_assert(std::is_same_v<bool, std::invoke_result_t<Callback, void>>, "Callback not return correct type");
		static_assert(std::is_invocable_v<Callback, void>, "Callback is not invocable");

		return func();
	}

	template <typename Resource>
	auto ResourceManager::GetResource(entt::id_type id) noexcept -> std::shared_ptr<Resource> {
		const auto entity = ResourceIndexer<Resource>::value();
		if(HasStorage<Resource>()) {
			const auto& storage = GetStorage<Resource>();
			const auto it = storage->find(id);
			if(it != storage.cend()) {
				return it->second;
			}
		}
		return std::shared_ptr<Resource>{};
	}

	template <typename Resource>
	auto ResourceManager::HasResource(entt::id_type id) noexcept -> bool {
		return HasStorage<Resource>() && GetSotrage<Resource>().count(id) ? true : false;
	}

	template <typename Resource>
	auto ResourceManager::RemoveResource(entt::id_type id) noexcept -> void {
		const auto entity = ResourceIndexer<Resource>::value();
		if(HasStorage<Resource>()) {
			auto& storage = GetStorage<Resource>();
			storage.erase(id);
			if(storage.empty()) {
				RemoveStorage<Resource>();
			}
		}
	}

	template <typename Resource>
	auto ResourceManager::AddStorage() -> Storage<Resource>& {
		const auto entity = ResourceIndexer<Resource>::value();
		return GetContext().m_Registry.emplace<Storage<Resource>>(entity);
	}

	template <typename Resource>
	auto ResourceManager::GetStorage() noexcept -> Storage<Resource>& {
		const auto entity = ResourceIndexer<Resource>::value();
		return GetContext().m_Registry.get<Storage<Resource>>(entity);
	}

	template <typename Resource>
	auto ResourceManager::HasStorage() noexcept -> bool {
		const auto entity = ResourceIndexer<Resource>::value();
		return GetContext().m_Registry.has<Storage<Resource>>(entity);
	}

	template <typename Resource>
	auto ResourceManager::RemoveStorage() noexcept -> void {
		const auto entity = ResourceIndexer<Resource>::value();
		GetContext().m_Registry.remove<Storage<Resource>>(entity);
	}

	template <typename Resource, typename Callback>
	auto ResourceManager::Each(Callback func) -> void
	{
		static_assert(std::is_same_v<void, std::invoke_result_t<Callback, entt::id_type, std::shared_ptr<Resource>>>, "Callback args incorrect");
		static_assert(std::is_invocable_v<Callback, entt::id_type, std::shared_ptr<Resource>>, "Callback is not invocable");

		const auto entity = ResourceIndexer<Resource>::value();
		if(HasStorage<Resource>()) {
			const auto& storage = GetStorage<Resource>();
			for(const auto& [key, value] : storage) {
				func(key, value);
			}
		}
	}
}

#endif // COMMON_RESOURCEMANAGER_IPP