#include <Include/PluginLog.hpp>

#include <Common/ModuleManager.hpp>
#include <Common/Xml.hpp>
#include <Common/Meta.hpp>
#include <Common/Util.hpp>

#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>

#ifdef HF_PLATFORM_WIN
	#include <spdlog/sinks/wincolor_sink.h>
#elif HF_PLATFORM_LINUX
	#include <spdlog/sinks/ansicolor_sink.h>
#endif

#include <filesystem>
#include <mutex>

namespace Helena
{
	PluginLog::~PluginLog() 
	{
		if(m_bAsync) {
			spdlog::drop_all();
			spdlog::shutdown();
		}
	}

	bool PluginLog::Initialize() {
		std::cout << "PluginLog initialize: " << this->m_pModuleManager->GetServiceName()  << std::endl;
		
		LOG_INFO("Hello logger");
		return true;
	}

	std::shared_ptr<spdlog::logger> PluginLog::GetLogger() 
	{
		// read log config once
		static std::once_flag flag;
		std::call_once(flag, &PluginLog::ConfigLogger, this);
		return spdlog::default_logger();
	}

	void PluginLog::ConfigLogger() 
	{
		const char* serviceName = m_pModuleManager->GetServiceName().c_str();
		const auto config = m_pModuleManager->GetDirectories()->GetConfigPath() + 
			Meta::ConfigLogger::ConfigFile();

		// load file
		pugi::xml_document xmlDoc;
		if(!xmlDoc.load_file(config.c_str(), pugi::parse_default | pugi::parse_comments)) {
			std::cerr << "[Info ] Parse file: " << config << " failed, default logger used!" << std::endl;
			return;
		}

		// find node
		const auto node = xmlDoc.find_child_by_attribute(Meta::ConfigLogger::Service(), Meta::ConfigLogger::Name(), serviceName);
		if(node.empty()) {
			std::cerr << "[Info ] Parse file: " << config << ", Node: " << Meta::ConfigService::Service()
				<< ", Service: " << serviceName << " not found, default logger used." << std::endl;
			return;
		}

		// get attributes
		const auto level	= spdlog::level::from_str(node.attribute(Meta::ConfigLogger::Level()).as_string("trace"));
		const auto format	= node.attribute(Meta::ConfigLogger::Format()).as_string("%^[%Y.%m.%d %H:%M:%S.%e][%@][%-8l] %v%$");
		const auto size		= node.attribute(Meta::ConfigLogger::Size()).as_uint();
		const auto thread	= node.attribute(Meta::ConfigLogger::Thread()).as_uint();
		std::string dir;

		{
			// get absolute path
			std::filesystem::path path{std::filesystem::absolute(node.attribute(Meta::ConfigLogger::Path()).as_string())};
			std::error_code error;

			// if path not empty check directory or create
			if(!path.empty())
			{
				if(!std::filesystem::exists(path, error) && !std::filesystem::create_directory(path, error)) {
					std::cerr << "[Error] Path: " << path << " create folder failed, default logger used!" << std::endl;
					return;
				}

				// add separator if not exist
				if(dir = path.string(); dir.back() != HF_SEPARATOR) {
					dir += HF_SEPARATOR;
				}

				// add service name
				dir += serviceName;
				dir += HF_SEPARATOR;
				dir += "log.txt";
			}
		}

		std::shared_ptr<spdlog::logger> logger;
		std::vector<spdlog::sink_ptr> sinks;

		try
		{
			if(thread && size) { // Multithread 
				m_bAsync = true;
				spdlog::init_thread_pool(size, thread);

				if(!dir.empty()) {
					auto dailySink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(dir, 23, 59);
					sinks.emplace_back(std::move(dailySink));
				}

			#ifdef HF_PLATFORM_WIN
				auto consoleSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
			#elif HF_PLATFORM_LINUX
				auto consoleSink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
			#else 
			#error Unknown platform
			#endif
				sinks.emplace_back(std::move(consoleSink));

			} else { // Single thread

				if(!dir.empty()) {
					auto dailySink = std::make_shared<spdlog::sinks::daily_file_sink_st>(dir, 23, 59);
					sinks.emplace_back(std::move(dailySink));
				}

			#ifdef HF_PLATFORM_WIN
				auto consoleSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>();
			#elif HF_PLATFORM_LINUX
				auto consoleSink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_st>();
			#else 
			#error Unknown platform
			#endif
				sinks.emplace_back(std::move(consoleSink));
			}

			logger = std::make_shared<spdlog::logger>(serviceName, sinks.begin(), sinks.end());
			spdlog::set_default_logger(std::move(logger));

		#ifdef HF_RELEASE
			spdlog::set_level(level);
			spdlog::set_pattern(format);
		#else 
			spdlog::set_level(spdlog::level::level_enum::trace);
			spdlog::set_pattern("%^[%Y.%m.%d %H:%M:%S.%e][%@][%-8l] %v%$");
		#endif

			spdlog::flush_on(spdlog::level::level_enum::trace);

		} catch(const spdlog::spdlog_ex& err) {
			std::cerr << "[Error] Config: " << config << " catch exception in spdlog, error: "
				<< err.what() << std::endl;
		}
	}
}