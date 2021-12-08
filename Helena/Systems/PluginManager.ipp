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
			if(plugin.m_State == EState::Init) {
				plugin.m_fnEnd();
			}
		}

		m_Plugins.clear();
	}

	inline auto PluginManager::Find(std::uint32_t hash) const noexcept {
		return std::find_if(m_Plugins.cbegin(), m_Plugins.cend(), [hash](const auto& plugin) noexcept {
			return plugin.m_Hash == hash;
		});
	}

	inline auto PluginManager::Find(std::uint32_t hash) noexcept {
		return std::find_if(m_Plugins.begin(), m_Plugins.end(), [hash](const auto& plugin) noexcept {
			return plugin.m_Hash == hash;
		});
	}

	[[nodiscard]] inline bool PluginManager::Load(const std::string_view path, const PluginName& name)
	{
		constexpr auto PathLength = 256;
		const auto hash = Hash::Get<std::uint32_t>(name);

		HELENA_ASSERT(Find(hash) == m_Plugins.cend(), "Plugin: {} already loaded!", name);
		HELENA_ASSERT((path.size() + name.GetSize() + sizeof(HELENA_SEPARATOR) + sizeof(HELENA_MODULE_EXTENSION)) <= PathLength);

		const auto module = path.empty() ? 
			Types::Format<PathLength>("{}{}", name, HELENA_MODULE_EXTENSION) :
			Types::Format<PathLength>("{}{}{}{}", path, HELENA_SEPARATOR, name, HELENA_MODULE_EXTENSION);
		const auto handle = static_cast<HELENA_MODULE_HANDLE>(HELENA_MODULE_LOAD(module.GetData()));
		if(!handle) {
			HELENA_MSG_ERROR("Plugin: {} load failed!", module);
			return false;
		}

		const auto fnInit = reinterpret_cast<fnPluginInit>(HELENA_MODULE_GETSYM(handle, KPluginInit));
		if(!fnInit) {
			HELENA_MODULE_UNLOAD(handle);
			HELENA_MSG_ERROR("Plugin: {} not loaded, function {} not found!", name, KPluginInit);
			return false;
		}

		const auto fnEnd = reinterpret_cast<fnPluginEnd>(HELENA_MODULE_GETSYM(handle, KPluginEnd));
		if(!fnEnd) {
			HELENA_MODULE_UNLOAD(handle);
			HELENA_MSG_ERROR("Plugin: {} not loaded, function {} not found!", name, KPluginEnd);
			return false;
		}

		Engine::SignalEvent<Events::PluginManager::Load>(std::ref(name));
		m_Plugins.emplace_back(name, handle, fnInit, fnEnd, hash, EState::Loaded);
		return true;
	}

	[[nodiscard]] inline bool PluginManager::Load(const PluginName& name) {
		return Load("", name);
	}

	[[nodiscard]] inline bool PluginManager::Init(const PluginName& name)
	{
		const auto hash = Hash::Get<std::uint32_t>(name);
		const auto it = Find(hash);
		HELENA_ASSERT(it != m_Plugins.cend(), "Plugin: {} not loaded!", name);
		HELENA_ASSERT(it->m_State == EState::Loaded, "Plugin: {} already initialized!", name);

		if(it != m_Plugins.cend() && it->m_State == EState::Loaded) 
		{
			it->m_State = EState::Init;
			it->m_fnInit(Engine::Context::Get());
			return true;
		}

		return false;
	}

	[[nodiscard]] inline bool PluginManager::End(const PluginName& name)
	{
		const auto hash = Hash::Get<std::uint32_t>(name);
		const auto it = Find(hash);
		HELENA_ASSERT(it != m_Plugins.cend(), "Plugin: {} not loaded!", name);
		HELENA_ASSERT(it->m_State == EState::Init, "Plugin: {} not initialized!", name);

		if(it != m_Plugins.cend() && it->m_State == EState::Init)
		{
			it->m_State = EState::Loaded;
			it->m_fnEnd();
			return true;
		}

		return false;
	}

	[[nodiscard]] inline bool PluginManager::Has(const PluginName& name) const noexcept {
		return Find(Hash::Get<std::uint32_t>(name)) != m_Plugins.cend();
	}

	[[nodiscard]] inline bool PluginManager::IsInitialized(const PluginName& name) const noexcept {
		const auto it = Find(Hash::Get<std::uint32_t>(name));
		return it != m_Plugins.cend() && it->m_State == EState::Init;
	}

	template <typename Func>
	void PluginManager::Each(Func callback) const 
	{
		for(std::size_t i = m_Plugins.size(); i; --i) {
			callback(m_Plugins[i - 1].m_Name);
		}
	}
}

#endif // HELENA_SYSTEMS_PLUGINMANAGER_IPP
