#pragma once

#ifndef COMMON_PLUGINMANAGER_IPP
#define COMMON_PLUGINMANAGER_IPP

namespace Helena
{
	inline auto PluginManager::GetContext() -> PluginManagerCtx& {
		static auto& ctx{Core::GetContextManager()->AddContext<PluginManagerCtx>()};
		return ctx;
	}


}

#endif // COMMON_PLUGINMANAGER_IPP