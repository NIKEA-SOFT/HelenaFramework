#ifndef COMMON_SERVICE_IPP
#define COMMON_SERVICE_IPP

#include <mutex>

#include "ModuleManager.hpp"

namespace Helena
{
    inline Service::Directories::Directories(std::string& configPath, std::string& modulePath, std::string& resourcePath)
        : m_PathConfig(std::move(configPath))
        , m_PathModule(std::move(modulePath))
        , m_PathResource(std::move(resourcePath)) {}

    [[nodiscard]] inline const std::string& Service::Directories::GetPathConfigs() const noexcept {
		return m_PathConfig;
	}	

    [[nodiscard]] inline const std::string& Service::Directories::GetPathModules() const noexcept {
		return m_PathModule;
	}

    [[nodiscard]] inline const std::string& Service::Directories::GetPathResources() const noexcept {
		return m_PathResource;
	}

    inline Service::Service(std::string& name, std::string& pathConfigs, std::string& pathModules, std::string& pathResources)
        : m_Name(std::move(name))
        , m_Directories(pathConfigs, pathModules, pathResources)
        , m_IsShutdown(false) {
        Service::m_Service = this;
    }

	[[nodiscard]] inline  const std::string& Service::GetName() const noexcept {
		return m_Name;
	}

	[[nodiscard]] inline const Service::Directories& Service::GetDirectories() const noexcept {
		return m_Directories;
	}

	[[nodiscard]] inline const std::unique_ptr<ModuleManager>& Service::GetModuleManager() const noexcept {
		return m_ModuleManager;
	}
    
    [[nodiscard]] inline const bool Service::IsShutdown() const noexcept {
        return m_IsShutdown;
    }

    inline void Service::Shutdown(const char* const filename, const std::size_t line, const std::string& msg)
    {
        static std::mutex mutex;
        std::lock_guard lock{mutex};
        if(!m_IsShutdown) {
            m_IsShutdown = true;
            if(filename && line && !msg.empty()) {
                m_ShutdownLog = fmt::format("[Error] [{}:{}] {}", filename, line, msg);
            }
        }
    }

    inline void Service::Initialize(const std::string_view moduleNames)
    {
    #if HF_PLATFORM == HF_PLATFORM_WIN
        SetConsoleTitle(m_Name.c_str());
        SetConsoleCtrlHandler(Service::CtrlHandler, TRUE);
    #elif HF_PLATFORM == HF_PLATFORM_LINUX
        signal(SIGHUP, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTERM, Service::SigHandler);
        signal(SIGSTOP, Service::SigHandler);
        signal(SIGINT, Service::SigHandler);
    #else
        #error Unknown platform
    #endif

        if(m_ModuleManager = std::make_unique<ModuleManager>(); !m_ModuleManager) {
            UTIL_CONSOLE_ERROR("Allocate memory for ModuleManager failed!");
            return;
        }

        m_ModuleManager->InitModules(this, moduleNames);
        
        const auto& plugns = m_ModuleManager->GetPlugins();
        for(const auto& plugin : plugns) 
        {
            if(IsShutdown() || !plugin.m_Plugin->Initialize()) {
                return;
            }
        }

        for(const auto& plugin : plugns)
        {
            if(IsShutdown() || !plugin.m_Plugin->Config()) {
                return;
            }
        }

        for(const auto& plugin : plugns)
        {
            if(IsShutdown() || !plugin.m_Plugin->Execute()) {
                return;
            }
        }

        for(;; Util::Sleep(1)) 
        {
            for(const auto& plugin : plugns)
            {
                if(IsShutdown() || !plugin.m_Plugin->Update()) {
                    return;
                }
            }
        }
    }

    inline void Service::Finalize() {
        m_ModuleManager->FreeModules();
        m_Service = nullptr;
    }

#if HF_PLATFORM == HF_PLATFORM_WIN
    inline BOOL WINAPI Service::CtrlHandler(DWORD)
    {
        static std::mutex mutex;
        std::lock_guard lock{mutex};
        if(m_Service) {
            m_Service->Shutdown();
            while(m_Service) {
                Util::Sleep(1000);
            }
        }

        return TRUE;
    }

    inline int Service::SEHHandler(unsigned int code, _EXCEPTION_POINTERS* pException) {
        // todo
        return EXCEPTION_EXECUTE_HANDLER;
    }
#elif HF_PLATFORM == HF_PLATFORM_LINUX
    void Service::SigHandler(int)
    {
        static std::mutex mutex;
        std::lock_guard lock{mutex};
        if(m_Service) {
            m_Service->Shutdown();
            while(m_Service) {
                Util::Sleep(1000);
            }
        }
    }
#endif
}

#endif // COMMON_SERVICE_IPP