#ifndef MODULE_MODULELOG_IPLUGINLOG_HPP
#define MODULE_MODULELOG_IPLUGINLOG_HPP

#include <Common/IPlugin.hpp>
#include <Common/Util.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <memory>

namespace Helena
{
	class IPluginLog : public IPlugin 
	{
	protected:
		virtual bool Finalize() = 0;

	public:
		template <typename... Args>
		void Log(spdlog::source_loc&& source, spdlog::level::level_enum level, const char* format, const Args&... args) 
		{
			if(const std::shared_ptr<spdlog::logger>& logger = GetLogger(); logger) {
				logger->log(source, level, format, args...);
			}
		}

	private:
		virtual std::shared_ptr<spdlog::logger> GetLogger() = 0;
	};

	#define LOG_TRACE(format, ...)		GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{Util::GetFileName(__FILE__), __LINE__, ""}, spdlog::level::trace, format, ##__VA_ARGS__);
	#define LOG_DEBUG(format, ...)		GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{Util::GetFileName(__FILE__), __LINE__, ""}, spdlog::level::debug, format, ##__VA_ARGS__);
	#define LOG_INFO(format, ...)		GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{Util::GetFileName(__FILE__), __LINE__, ""}, spdlog::level::info, format, ##__VA_ARGS__);
	#define LOG_WARN(format, ...)		GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{Util::GetFileName(__FILE__), __LINE__, ""}, spdlog::level::warn, format, ##__VA_ARGS__);
	#define LOG_ERROR(format, ...)		GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{Util::GetFileName(__FILE__), __LINE__, ""}, spdlog::level::err, format, ##__VA_ARGS__);
	#define LOG_CRITICAL(format, ...)	GetModuleManager()->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{Util::GetFileName(__FILE__), __LINE__, ""}, spdlog::level::critical, format, ##__VA_ARGS__);
}

#endif // MODULE_MODULELOG_IPLUGINLOG_HPP