#ifndef MODULE_MODULELOG_IPLUGINLOG_HPP
#define MODULE_MODULELOG_IPLUGINLOG_HPP

#include <Common/IPlugin.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <memory>

namespace Helena
{	
	// Get filename from compile-time
	constexpr const char* spdlog_filename(std::string_view filename) {
		constexpr std::string_view find = "\\/";
		const auto offset = filename.find_last_of(find);
		return offset == std::string_view::npos ? filename.data() : (filename.data() + offset + 1);
	}

	class IPluginLog : public IPlugin
	{
	public:
		template <typename... Args>
		void Log(spdlog::source_loc source, spdlog::level::level_enum level, const char* format, const Args&... args) {
			if(const std::shared_ptr<spdlog::logger>& logger = GetLogger(); logger) {
				logger->log(source, level, format, args...);
			}
		}

	private:
		virtual std::shared_ptr<spdlog::logger> GetLogger() = 0;
	};

	#define LOG_TRACE(format, ...)		m_pModuleManager->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, ""}, spdlog::level::trace, format, ##__VA_ARGS__);
	#define LOG_DEBUG(format, ...)		m_pModuleManager->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, ""}, spdlog::level::debug, format, ##__VA_ARGS__);
	#define LOG_INFO(format, ...)		m_pModuleManager->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, ""}, spdlog::level::info, format, ##__VA_ARGS__);
	#define LOG_WARN(format, ...)		m_pModuleManager->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, ""}, spdlog::level::warn, format, ##__VA_ARGS__);
	#define LOG_ERROR(format, ...)		m_pModuleManager->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, ""}, spdlog::level::err, format, ##__VA_ARGS__);
	#define LOG_CRITICAL(format, ...)	m_pModuleManager->GetPlugin<IPluginLog>()->Log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, ""}, spdlog::level::critical, format, ##__VA_ARGS__);
}

#endif // MODULE_MODULELOG_IPLUGINLOG_HPP