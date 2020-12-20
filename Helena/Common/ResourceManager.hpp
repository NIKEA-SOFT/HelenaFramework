#pragma once

#ifndef COMMON_RESOURCEMANAGER_HPP
#define COMMON_RESOURCEMANAGER_HPP

namespace Helena
{
	class ResourceManager final 
	{
		template <typename Resource>
		struct ResourceIndexer final {
			[[nodiscard]] static auto value() noexcept {
				static auto value = ResourceManager::GetResourceEntity(Hash::HashType<Resource>::value());
				return value;
			}
		};

		struct ResourceManagerCtx {
			entt::registry m_Registry;
			std::unordered_map<entt::id_type, entt::entity> m_ResourceIndexes;
		};

	private:
		static auto GetContext() -> ResourceManagerCtx&;
		static auto GetResourceEntity(entt::id_type name) noexcept -> entt::entity;

	public:
		ResourceManager() = delete;
		~ResourceManager() = delete;
		ResourceManager(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager& operator=(ResourceManager&&) = delete;

		template <typename Resource>
		using Storage = std::unordered_map<entt::id_type, std::shared_ptr<Resource>>;

		template <typename Resource, typename... Args>
		static auto AddResource(entt::id_type id, Args&&... args) -> std::shared_ptr<Resource>;

		template <typename Callback>
		static auto AddResource(Callback func) -> bool;

		template <typename Resource>
		static auto GetResource(entt::id_type id) noexcept -> std::shared_ptr<Resource>;

		template <typename Resource>
		static auto HasResource(entt::id_type id) noexcept -> bool;

		template <typename Resource>
		static auto RemoveResource(entt::id_type id) noexcept -> void;

		template <typename Resource>
		static auto AddStorage() -> Storage<Resource>&;

		template <typename Resource>
		static auto GetStorage() noexcept -> Storage<Resource>&;

		template <typename Resource>
		static auto HasStorage() noexcept -> bool;

		template <typename Resource>
		static auto RemoveStorage() noexcept -> void;

		template <typename Resource, typename Callback>
		static auto Each(Callback func) -> void;
	};
}

#include <Common/ResourceManager.ipp>

#endif // COMMON_RESOURCEMANAGER_HPP