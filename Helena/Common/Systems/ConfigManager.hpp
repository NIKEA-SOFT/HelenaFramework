#ifndef COMMON_SYSTEMS_CONFIGMANAGER_HPP
#define COMMON_SYSTEMS_CONFIGMANAGER_HPP

namespace Helena::Systems
{
	class ConfigManager final 
	{
		using map_index_t		= robin_hood::unordered_flat_map<entt::id_type, std::size_t>;
		using vec_storage_t		= std::vector<entt::any>;

		template <typename Resource>
		struct ResourceIndex {
			[[nodiscard]] static auto GetIndex(map_index_t& container) -> std::size_t;
		};

		template <typename Resource, typename Propertie>
		struct PropertieIndex {
			[[nodiscard]] static auto GetIndex(map_index_t& container) -> std::size_t;
		};

		//struct Storage {
		//	entt::any m_Instance;
		//	vec_propertie_t m_Properties;
		//	map_index_t	m_PropertieIndexes;
		//};

		//using vec_storage_t	= std::vector<Storage>;

	public:
		using ResourceID = entt::id_type;

	public:

		ConfigManager() = default;
		~ConfigManager() = default;
		ConfigManager(const ConfigManager&) = delete;
		ConfigManager(ConfigManager&&) noexcept = delete;
		ConfigManager& operator=(const ConfigManager&) = delete;
		ConfigManager& operator=(ConfigManager&&) noexcept = delete;

		template <typename Resource, typename... Args>
		auto AddResource([[maybe_unused]] Args&&... args) -> void;

		template <typename Resource>
		auto HasResource() noexcept -> bool;

		template <typename Resource>
		auto GetResource() noexcept -> Resource&;

		template <typename Resource>
		auto RemoveResource() noexcept -> void;

	private:
		vec_storage_t	m_Resources;
		vec_storage_t	m_Properties;
		map_index_t		m_ResourceIndexes;
		map_index_t		m_PropertieIndexes;
	};
}

#include <Common/Systems/ConfigManager.ipp>

#endif // COMMON_SYSTEMS_CONFIGMANAGER_HPP