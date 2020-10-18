#ifndef MODULELOG_IPLUGINLOG_HPP
#define MODULELOG_IPLUGINLOG_HPP

#include <spdlog/spdlog.h>

#include <Common/IPlugin.hpp>
#include <Common/Util.hpp>

namespace Helena
{
	class IPluginLog : public IPlugin 
	{
	protected:
		

	public:
		template <typename... Args>
		void Log(const spdlog::source_loc& source, spdlog::level::level_enum level, const char* format, const Args&... args) 
		{
			if(const std::shared_ptr<spdlog::logger>& logger = GetLogger(); logger) {
				logger->log(source, level, format, args...);
			}
		}

	private:
		virtual std::shared_ptr<spdlog::logger> GetLogger() = 0;
	};

	#define LOG_TRACE(format, ...)		GetService()->GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{UTIL_FILE_LINE, ""}, spdlog::level::trace, format, ##__VA_ARGS__);
	#define LOG_DEBUG(format, ...)		GetService()->GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{UTIL_FILE_LINE, ""}, spdlog::level::debug, format, ##__VA_ARGS__);
	#define LOG_INFO(format, ...)		GetService()->GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{UTIL_FILE_LINE, ""}, spdlog::level::info, format, ##__VA_ARGS__);
	#define LOG_WARN(format, ...)		GetService()->GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{UTIL_FILE_LINE, ""}, spdlog::level::warn, format, ##__VA_ARGS__);
	#define LOG_ERROR(format, ...)		GetService()->GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{UTIL_FILE_LINE, ""}, spdlog::level::err, format, ##__VA_ARGS__);
	#define LOG_CRITICAL(format, ...)	GetService()->GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{UTIL_FILE_LINE, ""}, spdlog::level::critical, format, ##__VA_ARGS__);
}

#endif // MODULELOG_IPLUGINLOG_HPP