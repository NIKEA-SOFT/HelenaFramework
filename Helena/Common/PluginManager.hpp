#pragma once

#ifndef COMMON_PLUGINMANAGER_HPP
#define COMMON_PLUGINMANAGER_HPP

namespace Helena 
{
	class PluginManager final 
	{
		struct PluginManagerCtx {
			entt::registry m_Registry;
		};

	public:
		PluginManager() = delete;
		~PluginManager() = delete;
		PluginManager(const PluginManager&) = delete;
		PluginManager(PluginManager&&) = delete;
		PluginManager& operator=(const PluginManager&) = delete;
		PluginManager& operator=(PluginManager&&) = delete;

	private:
		static auto GetContext() -> PluginManagerCtx&;
	};
}

#include <Common/PluginManager.ipp>

#endif // COMMON_PLUGINMANAGER_HPP