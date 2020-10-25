#ifndef COMMON_SERVICE_HPP
#define COMMON_SERVICE_HPP

#include <string>
#include <memory>
#include <filesystem>
#include <thread>
#include <vector>
#include <array>
#include <unordered_map>
#include <mutex>

#include "Util.hpp"

#define HF_SHUTDOWN(...)        GetService()->Shutdown(HF_FILE_LINE, ##__VA_ARGS__);

int main(int, char**);

namespace Helena
{

    class ModuleManager;
    class Service final {
        friend int ::main(int, char**);

        // Service directories
        class Directories final {
        public:
            explicit Directories(std::string& configPath, std::string& modulePath, std::string& resourcePath);
            ~Directories() = default;
            Directories(const Directories&) = delete;
            Directories(Directories&&) = delete;
            Directories& operator=(const Directories&) = delete;
            Directories& operator=(Directories&&) = delete;

            /**
            * @brief    Get the path to module config files
            * @return   @code{.cpp} const std::string& @endcode
            */
            [[nodiscard]] const std::string& GetPathConfigs() const noexcept;

            /**
            * @brief    Get the path to modules
            * @return   @code{.cpp} const std::string& @endcode
            */
            [[nodiscard]] const std::string& GetPathModules() const noexcept;

            /**
            * @brief    Get the path to resources (configs/resource of services)
            * @return   @code{.cpp} const std::string& @endcode
            */
            [[nodiscard]] const std::string& GetPathResources() const noexcept;

        private:
            std::string m_PathConfig;       // service configs of modules path
            std::string m_PathModule;       // service modules path
            std::string m_PathResource;     // service resources path
        };
        
    public:
        explicit Service(std::string& name, std::string& pathConfigs, std::string& pathModules, std::string& pathResources);
        ~Service() = default;
        Service(const Service&) = delete;
        Service(Service&&) = delete;
        Service& operator=(const Service&) = delete;
        Service& operator=(Service&&) = delete;
        
    public:
        /*
        * @brief    Get service name
        * @return   @code{.cpp} const std::string& @endcode
        */
        [[nodiscard]] const std::string& GetName() const noexcept;

        /**
        * @brief    Get service directories
        * @return   @code{.cpp} const Directories* @endcode
        */
        [[nodiscard]] const Service::Directories& GetDirectories() const noexcept;

        /**
        * @brief    Get service module manager
        * @return   @code{.cpp} const std::unique_ptr<ModuleManager>& @endcode
        */
        [[nodiscard]] const std::unique_ptr<ModuleManager>& GetModuleManager() const noexcept;

        /**
        * @brief    Get framework shutdown status
        * @return   @code{.cpp} const bool @endcode
        */
        [[nodiscard]] const bool IsShutdown() const noexcept;

        /**
        * @brief    Shutdown framework and unload modules
        *
        * @param    location    Result of HF_FILE_LINE from Util.hpp
        *
        * @note     Call Shutdown() for stop framework and unload modules without error.
        *           This function allows you to shutdown the framework correctly.
        */
        void Shutdown(const Util::Internal::Location& location);

        /**
        * @brief    Shutdown framework and unload modules
        * 
        * @tparam   args        Type of args for format
        *
        * @param    location    Result of HF_FILE_LINE from Util.hpp
        * @param    message     Shutdown reason
        * @param    args        Args for format
        *
        * @note     Call Shutdown() for stop framework and unload modules without error.
        *           This function allows you to shutdown the framework correctly.
        */
        template <typename... Args>
        void Shutdown(const Util::Internal::Location& location, const std::string_view message, [[maybe_unused]] Args&&... args);

    private:
        void Initialize(const std::string_view moduleNames);
        void Finalize();
        
    private:
    #if HF_PLATFORM == HF_PLATFORM_WIN
        static BOOL WINAPI CtrlHandler(DWORD);
        static int SEHHandler(unsigned int code, _EXCEPTION_POINTERS* pException);
    #elif HF_PLATFORM == HF_PLATFORM_LINUX
        static void SigHandler(int);
    #endif

    private:
        std::string m_Name;         // service name
        Directories m_Directories;  // service directories
        std::string m_ShutdownLog;  // service shutdown log
        std::unique_ptr<ModuleManager> m_ModuleManager; // service module manager
        //EventManager    m_EventManager;         // service event manager
        bool m_IsShutdown; // service shutdown state

        static inline Service* m_Service;
    };
}

#include "Service.ipp"

#endif // COMMON_SERVICE_HPP