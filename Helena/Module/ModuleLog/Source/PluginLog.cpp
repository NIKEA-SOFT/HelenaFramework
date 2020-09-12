#include <Include/PluginLog.hpp>

#include <Common/ModuleManager.hpp>
#include <Common/Xml.hpp>
#include <Common/Meta.hpp>

#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>

#if HF_PLATFORM == HF_PLATFORM_WIN
	#include <spdlog/sinks/wincolor_sink.h>
#elif HF_PLATFORM == HF_PLATFORM_LINUX
	#include <spdlog/sinks/ansicolor_sink.h>
#endif

#include <filesystem>
#include <mutex>

namespace Helena
{
	bool PluginLog::Finalize()
	{
		m_pLogger.reset();
		m_pThreadPool.reset();
		spdlog::drop_all();
		spdlog::shutdown();
		return true;
	}

	std::shared_ptr<spdlog::logger> PluginLog::GetLogger() {
		static std::once_flag flag;
		std::call_once(flag, &PluginLog::Configure, this);
		return m_pLogger;
	}

	/* @brief Parse service from Log config */
	void PluginLog::Configure()
	{
		const char* serviceName = GetModuleManager()->GetServiceName().c_str();
		const auto config = GetModuleManager()->GetDirectories()->GetConfigPath() + Meta::ConfigLogger::ConfigFile();

		// load file
		pugi::xml_document xmlDoc;
		if(!xmlDoc.load_file(config.c_str(), 
			pugi::parse_default | pugi::parse_comments)) {
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


		/*
		const auto flushTime = node.attribute(Meta::ConfigLogger::FlushTime())
			.as_ullong(0); */

		try
		{
		#ifdef HF_RELEASE
			const auto level = spdlog::level::from_str(node.attribute(Meta::ConfigLogger::Level()).as_string("trace"));
			if(level == spdlog::level::level_enum::off) {
				m_pLogger->set_level(level);
				return;
			}
		#endif

			const auto buffer = node.attribute(Meta::ConfigLogger::Buffer()).as_ullong();
			const auto threads = node.attribute(Meta::ConfigLogger::Threads()).as_ullong();
			std::string_view path = node.attribute(Meta::ConfigLogger::Path()).as_string();
			if(buffer && threads) {
				SetupLoggerMT(path, buffer, threads);
			} else {
				SetupLoggerST(path);
			}
	
		#ifdef HF_RELEASE
			const auto format = node.attribute(Meta::ConfigLogger::Format())
				.as_string("%^[%Y.%m.%d %H:%M:%S.%e][%@][%-8l] %v%$");

			m_pLogger->set_level(level);
			m_pLogger->set_pattern(format);

			if(!path.empty())
			{
				const auto flushLevel = spdlog::level::from_str(node.attribute(Meta::ConfigLogger::FlushLevel())
					.as_string("off"));

				m_pLogger->flush_on(flushLevel);
				/*
				if(flushTime && flushLevel != spdlog::level::level_enum::trace) {
					spdlog::flush_every(std::chrono::seconds(flushTime));
				}*/
			}

		#else 
			m_pLogger->set_level(spdlog::level::level_enum::trace);
			m_pLogger->set_pattern("%^[%Y.%m.%d %H:%M:%S.%e][%@][%-8l] %v%$");

			if(!path.empty()) {
				m_pLogger->flush_on(spdlog::level::level_enum::trace);
			}
		#endif

			spdlog::register_logger(m_pLogger);
		} catch(const spdlog::spdlog_ex& err) {
			std::cerr << "[Error] Config: " << config 
				<< " catch exception in spdlog, error: " << err.what() << std::endl;
		}
	}

	/**
	* @brief	Get full path with log file naem
	* 
	* @param	path	-> Path to out logs folder
	* 
	* @return	@code{.cpp} std::string @endcode
	*/
	std::string PluginLog::GetFileLog(const std::string_view path) 
	{
		std::error_code error;
		const auto logPath = std::filesystem::absolute(path, error);
		std::string logFile;

		if(!path.empty()) 
		{
			if(!std::filesystem::exists(logPath, error) && !std::filesystem::create_directory(logPath, error)) {
				std::cerr << "[Error] Path: " << logPath 
					<< " create folder failed, default logger used!" << std::endl;
			} else {
				if(logFile = logPath.string(); logFile.back() != HF_SEPARATOR) {
					logFile += HF_SEPARATOR;
				}

				logFile += GetModuleManager()->GetServiceName();
				logFile += HF_SEPARATOR;
				logFile += "log.txt";
			}
		}

		return logFile;
	}

	/**
	* @brief	Setup single-threaded logger
	* 
	* @param	Path	-> Path for storing log
	*/
	void PluginLog::SetupLoggerST(const std::string_view path)
	{
		std::vector<spdlog::sink_ptr> sinks;
		if(const auto logFile = GetFileLog(path); !logFile.empty()) {
			auto dailySink = std::make_shared<spdlog::sinks::daily_file_sink_st>(logFile, 23, 59);
			sinks.emplace_back(std::move(dailySink));
		}

	#if HF_PLATFORM == HF_PLATFORM_WIN
		auto consoleSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>();
	#elif HF_PLATFORM == HF_PLATFORM_LINUX
		auto consoleSink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_st>();
	#else 
		#error Unknown platform
	#endif
		sinks.emplace_back(std::move(consoleSink));
		m_pLogger = std::make_shared<spdlog::logger>(GetModuleManager()->GetServiceName(), sinks.begin(), sinks.end());
	}

	/***
	* @brief	Setup multi-threaded logger
	* 
	* @param	path	-> Path for storing logs
	* @param	buffer	-> Msg buffer size per thread
	* @param	threads	-> Count of threads
	*/
	void PluginLog::SetupLoggerMT(const std::string_view path, const std::size_t buffer, const std::size_t threads)
	{
		std::vector<spdlog::sink_ptr> sinks;
		if(const auto logFile = GetFileLog(path); !logFile.empty()) {
			auto dailySink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logFile, 23, 59);
			sinks.emplace_back(std::move(dailySink));
		}

	#if HF_PLATFORM == HF_PLATFORM_WIN
		auto consoleSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
	#elif HF_PLATFORM == HF_PLATFORM_LINUX
		auto consoleSink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
	#else 
		#error Unknown platform
	#endif

		m_pThreadPool = std::make_shared<spdlog::details::thread_pool>(buffer, threads);
		sinks.emplace_back(std::move(consoleSink));
		m_pLogger = std::make_shared<spdlog::async_logger>(GetModuleManager()->GetServiceName(),
			sinks.begin(), sinks.end(), m_pThreadPool);
	}
}