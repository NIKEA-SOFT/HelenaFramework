#ifndef COMMON_SYSTEMS_CONFIGMANAGER_HPP
#define COMMON_SYSTEMS_CONFIGMANAGER_HPP

namespace Helena::Systems
{
	class ConfigManager final 
	{
		using map_index_t = robin_hood::unordered_flat_map<entt::id_type, std::size_t>;
		using vec_any_t = std::vector<entt::any>;

		template <typename Resource>
		struct ResourceIndex {
			[[nodiscard]] static auto GetIndex(map_index_t& container) -> std::size_t;
		};

		template <typename Resource, typename Propertie>
		struct PropertieIndex {
			[[nodiscard]] static auto GetIndex(map_index_t& container) -> std::size_t;
		};

	public:
		using ResourceID = entt::id_type;

		template <typename Resource>
		struct Storage {
			template <typename... Args>
			Storage([[maybe_unused]] Args&&... args) : m_Instance{std::forward<Args>(args)...} {}
			~Storage() = default;

			Resource	m_Instance;
			vec_any_t	m_Properties;
			map_index_t	m_PropertieIndexes;
		};

	public:

		ConfigManager() = default;
		~ConfigManager() = default;
		ConfigManager(const ConfigManager&) = default;
		ConfigManager(ConfigManager&&) noexcept = default;
		ConfigManager& operator=(const ConfigManager&) = delete;
		ConfigManager& operator=(ConfigManager&&) noexcept = delete;

		//template <typename Resource, typename Key, typename... Args>
		//auto AddResource(const Key& key, Args&&... args) -> decltype(auto) {

		//}

		template <typename Resource, typename... Args>
		auto AddResource(Args&&... args) -> void;

		template <typename Resource>
		auto GetResource() -> Resource*;

		template <typename Resource>
		auto RemoveResource() -> void;

	private:
		vec_any_t m_Storage;
		map_index_t m_ResourceIndexes;
	};
}

#include <Common/Systems/ConfigManager.ipp>

#endif // COMMON_SYSTEMS_CONFIGMANAGER_HPP