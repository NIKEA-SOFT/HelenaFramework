#ifndef HELENA_SYSTEMS_PLUGINMANAGER_IPP
#define HELENA_SYSTEMS_PLUGINMANAGER_IPP

#include <Helena/Systems/PluginManager.hpp>
#include <Helena/Types/Hash.hpp>

namespace Helena::Systems
{
	inline PluginManager::~PluginManager() 
	{
		for(auto& plugin : m_Plugins) 
		{
			const auto fn = reinterpret_cast<void (*)()>(HELENA_MODULE_GETSYM(plugin.m_Handle, fnPluginEnd));
			if(fn) {
				fn();
			}
		}

		m_Plugins.clear();
	}

	inline auto PluginManager::Find(std::uint32_t hash) const noexcept 
	{
		return std::find_if(m_Plugins.cbegin(), m_Plugins.cend(), [hash](const auto& plugin) noexcept {
			return plugin.m_Hash == hash;
		});
	}

	[[nodiscard]] inline bool PluginManager::Create(const FixedBuffer& name)
	{
		const auto hash = Hash::Get<std::uint32_t>(name);
		HELENA_ASSERT(Find(hash) == m_Plugins.cend(), "Plugin: {} already exist!", name);
		if(const auto handle = static_cast<HELENA_MODULE_HANDLE>(HELENA_MODULE_LOAD(name.GetData())); !handle) {
			HELENA_MSG_ERROR("Load plugin: {} failed!", name);
			return false;
		} else if(const auto fn = reinterpret_cast<void (*)(std::shared_ptr<Engine::Context>)>(HELENA_MODULE_GETSYM(handle, fnPluginInit)); !fn) {
			HELENA_MSG_ERROR("Load plugin: {}, function: PluginInit not found!", name);
			return false;
		} else {
			fn(Engine::Context::Get());
			m_Plugins.emplace_back(name, handle, hash);
			Engine::SignalEvent<Events::PluginManager::Create>(std::ref(name));
			return true;
		}
	}

	inline void PluginManager::Remove(const FixedBuffer& name)
	{
		const auto hash = Hash::Get<std::uint32_t>(name);
		const auto it = Find(hash);
		if(it != m_Plugins.cend()) 
		{
			const auto fn = reinterpret_cast<void (*)()>(HELENA_MODULE_GETSYM(it->m_Handle, fnPluginEnd));
			if(fn) {
				fn();
			}

			Engine::SignalEvent<Events::PluginManager::Remove>(std::ref(name));
			m_Plugins.erase(it);
		}
	}
}

#endif // HELENA_SYSTEMS_PLUGINMANAGER_IPP
