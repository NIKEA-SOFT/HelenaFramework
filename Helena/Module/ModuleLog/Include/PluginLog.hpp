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
		void Configure();
		std::string GetFileLog(const std::string_view path);
		void SetupLoggerST(const std::string_view path);
		void SetupLoggerMT(const std::string_view path, const std::size_t buffer, const std::size_t threads);

	private:
		std::shared_ptr<spdlog::logger> m_pLogger;
		std::shared_ptr<spdlog::details::thread_pool> m_pThreadPool;
		ModuleManager* m_pModuleManager;
		bool m_bAsync;
	};
}

#endif // MODULE_MODULELOG_PLUGINLOG_HPP