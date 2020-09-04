#ifndef __MODULE_MODULELOG_PLUGINLOG_HPP__
#define __MODULE_MODULELOG_PLUGINLOG_HPP__

#include <Interface/IPluginLog.hpp>

namespace Helena
{
	class ModuleManager;
	class PluginLog : public IPluginLog 
	{
	public:
		PluginLog(ModuleManager* pModuleManager) 
		: m_pModuleManager(pModuleManager) {}

	public:
		bool Initialize() override;

	private:
		ModuleManager* m_pModuleManager;
	};
}

#endif // __MODULE_MODULELOG_PLUGINLOG_HPP__