#ifndef HELENA_SYSTEMS_JOBMANAGER_IPP
#define HELENA_SYSTEMS_JOBMANAGER_IPP

#include <Helena/Systems/JobManager.hpp>
#include <Helena/Types/Hash.hpp>

namespace Helena::Systems
{
	inline auto PluginManager::Find(const FixedBuffer& name) const noexcept 
	{
		const auto hash = Hash::Get<std::uint32_t>(name);
		return std::find_if(m_Plugins.cbegin(), m_Plugins.cend(), [hash](const auto& plugin) noexcept {
			return plugin.m_Hash == hash;
		});
	}

	inline void PluginManager::Create(const FixedBuffer& name) 
	{
		HELENA_ASSERT(Find(name) != m_Plugins.cend(), "Plugin: {} already exist!", name);
		//m_Plugins.emplace_back()
	}
}

#endif // HELENA_SYSTEMS_JOBMANAGER_IPP
