#ifndef COMMON_SYSTEMS_CONFIGMANAGER_HPP
#define COMMON_SYSTEMS_CONFIGMANAGER_HPP

namespace Helena::Systems
{
	class ConfigManager final 
	{
		template <typename Resource>
		struct ResourceIndex {
			[[nodiscard]] static auto GetIndex() -> std::size_t;
		};

	public:


		ConfigManager() = default;
		~ConfigManager() = default;
		ConfigManager(const ConfigManager&) = default;
		ConfigManager(ConfigManager&&) noexcept = default;
		ConfigManager& operator=(const ConfigManager&) = delete;
		ConfigManager& operator=(ConfigManager&&) noexcept = delete;

		template <typename Resource, typename... Args>
		auto AddResource(Args&&... args) -> Resource&;

	public:
		std::vector<entt::any> m_Storage;
		robin_hood::unordered_flat_map<entt::id_type, std::size_t> m_Indexes;
	};
}

#include <Common/Systems/ConfigManager.ipp>

#endif // COMMON_SYSTEMS_CONFIGMANAGER_HPP