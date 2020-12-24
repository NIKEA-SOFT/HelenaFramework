#pragma once

#ifndef COMMON_RESOURCEMANAGER_HPP
#define COMMON_RESOURCEMANAGER_HPP

namespace Helena
{
	class ResourceManager final 
	{
		template <typename Type>
		struct StorageIndexer final {
			[[nodiscard]] static auto GetIndex() noexcept {
				static auto value = ResourceManager::GetStorageIndex(entt::type_hash<Type>::value());
				return value;
			}
		};
		
		template <typename Type>
		struct ResourceNull {
			[[nodiscard]] static auto& Get() noexcept {
				static Type instance{};
				return instance;
			}
		};

		struct ResourceManagerCtx {
			entt::registry m_Registry;
			std::unordered_map<entt::id_type, entt::entity> m_StorageIndexes;
		};

	private:
		static auto GetContext() -> ResourceManagerCtx&;
		static auto GetStorageIndex(entt::id_type key) noexcept -> entt::entity;

	public:
		ResourceManager() = delete;
		~ResourceManager() = delete;
		ResourceManager(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) noexcept = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager& operator=(ResourceManager&&) noexcept = delete;


		template <typename Type>
		using Resource = std::shared_ptr<Type>;

		template <typename Type>
		class Storage final 
		{
			using ResourceMap = std::unordered_map<entt::id_type, Resource<Type>>;

		public:
			Storage() = default;
			~Storage() = default;
			Storage(const Storage&) = default;
			Storage(Storage&&) noexcept = default;
			Storage& operator=(const Storage&) = default;
			Storage& operator=(Storage&&) noexcept = default;

			template <typename... Args>
			auto CreateResource(Args&&... args) -> Resource<Type>&;
			template <typename... Args>
			auto AddResource(entt::id_type key, Args&&... args) -> Resource<Type>&;
			auto GetResource(entt::id_type key) noexcept -> Resource<Type>&;
			auto GetResources() noexcept -> const ResourceMap&;
			auto RemoveResource(entt::id_type key = entt::type_hash<Type>::value()) -> void;
			template <typename Callback>
			auto EachResources(Callback func) -> void;
		private:
			ResourceMap m_Resources;
		};

		template <typename Type>
		using Container = std::shared_ptr<Storage<Type>>;

		template <typename Type>
		static auto SetStorage(const Container<Type>&) -> void;
		template <typename Type>
		static auto CreateStorage() -> Container<Type>&;
		template <typename Type>
		static auto ExtractStorage() noexcept -> Container<Type>;
		template <typename Type>
		static auto HasStorage() noexcept -> bool;
		template <typename Type>
		static auto GetStorage() noexcept -> Container<Type>&;
		template <typename Type>
		static auto RemoveStorage() -> void;
	};
}

#include <Common/ResourceManager.ipp>

#endif // COMMON_RESOURCEMANAGER_HPP