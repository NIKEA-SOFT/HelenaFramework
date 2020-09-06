#ifndef MODULE_MODULELOG_PLUGINLOG_HPP
#define MODULE_MODULELOG_PLUGINLOG_HPP

#include <Interface/IPluginLog.hpp>

namespace Helena
{
	class ModuleManager;
	class PluginLog : public IPluginLog 
	{
	protected:
		bool Initialize() override;

	public:
		PluginLog(ModuleManager* pModuleManager) 
		: m_pModuleManager(pModuleManager)
		, m_bAsync(false) {}
		~PluginLog();

	private:
		std::shared_ptr<spdlog::logger> GetLogger() override;

	private:
		void ConfigLogger();

	private:
		ModuleManager* m_pModuleManager;
		bool m_bAsync;
	};
}

#endif // MODULE_MODULELOG_PLUGINLOG_HPP