#ifndef COMMON_MODULEMANAGER_HPP
#define COMMON_MODULEMANAGER_HPP

#include <array>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>

#include "IPlugin.hpp"
#include "Util.hpp"

namespace Helena
{
    // State of module for EntryPoint
    enum class EModuleState : uint8_t {
        Init,
        Free
    };

    class ModuleManager final
    {
        friend void HelenaFramework(std::string&, std::string&&, std::string&&, std::string&&, std::vector<std::string>&);

        // Storage directories
        class Directories final
        {
        public:
            Directories(std::string& configPath, std::string& modulePath, std::string& resourcePath)
                : m_ConfigPath(std::move(configPath))
                , m_ModulePath(std::move(modulePath))
                , m_ResourcePath(std::move(resourcePath)) {}
            ~Directories() = default;
            Directories(const Directories&) = delete;
            Directories(Directories&&) = delete;
            Directories& operator=(const Directories&) = delete;
            Directories& operator=(Directories&&) = delete;

            /**
            * @brief    Get the path to module config files
            * @return   @code{.cpp} const std::string& @endcode
            */
            [[nodiscard]] const std::string& GetConfigPath() const noexcept {
                return m_ConfigPath;
            }

            /**
            * @brief    Get the path to modules
            * @return   @code{.cpp} const std::string& @endcode
            */
            [[nodiscard]] const std::string& GetModulePath() const noexcept {
                return m_ModulePath;
            }

            /**
            * @brief    Get the path to resources (configs/resource of services)
            * @return   @code{.cpp} const std::string& @endcode
            */
            [[nodiscard]] const std::string& GetResourcePath() const noexcept {
                return m_ResourcePath;
            }

        private:
            std::string m_ConfigPath;
            std::string m_ModulePath;
            std::string m_ResourcePath;
        };

        // Storage dll/so info
        struct Module
        {
            // EntryPoint
            using ModuleEP = void (*)(ModuleManager*, EModuleState);

            Module(std::string name)
                : m_Name(std::move(name))
                , m_pHandle(nullptr)
                , m_pMain(nullptr) {}

            std::string m_Name;             // Module name
            HF_MODULE_HANDLE m_pHandle;     // Ptr on handle
            ModuleEP m_pMain;               // Ptr on EP
        };

        // Cache pointer on Plugin for optimization
        template <typename Type>
        struct PluginPtr {
            inline static Type* m_pPlugin{nullptr};
        };

    public:
        /**
        * @brief    Ctor of ModuleManager
        *
        * @param    serviceName     Application name taken from the service file name
        * @param    configPath      Path to config files of modules
        * @param    modulePath      Path to files of modules (dll/so)
        * @param    resourcePath    Path to resources of service
        */
        explicit ModuleManager(std::string& serviceName, std::string& configPath, std::string& modulePath, std::string& resourcePath)
            : m_Directories(configPath, modulePath, resourcePath)
            , m_ServiceName(std::move(serviceName))
            , m_bFinish(false) {
            m_pModuleManager = this;
        }

        ~ModuleManager() = default;
        ModuleManager(const ModuleManager&) = delete;
        ModuleManager(ModuleManager&&) noexcept = delete;
        ModuleManager& operator=(const ModuleManager&) = delete;
        ModuleManager& operator=(ModuleManager&&) noexcept = delete;

        /**
        * @brief    Shutdown Framework and unload modules
        *
        * @param    filename    Result of Util::GetFileName(__FILE__)
        * @param    line        Result of __LINE__
        * @param    msg         Message of error
        *
        * @note     Call Shutdown() for stop framework and unload modules without error.
        *           This function allows you to shutdown the framework correctly.
        */
        void Shutdown(const char* const filename = nullptr, const std::size_t line = 0, const std::string& msg = {})
        {
            static std::mutex mutex;
            const std::lock_guard<std::mutex> lock{mutex};

            if(!m_bFinish)
            {
                if(filename && line && !msg.empty()) {
                    m_LastError = fmt::format("[Error] [{}:{}] {}", filename, line, msg);
                }

                m_bFinish = true;
            }
        }

        /**
        * @brief    Create instance of plugin into the map
        *
        * @tparam   Base    Type of abstract class inherited from IPlugin
        * @tparam   Plugin  Type of plugin class inherited from Base
        * @param    args    Arguments for ctor of Plugin
        *
        * @note     If it was not possible to create an instance of the class,
        *           then the framework will finalize with error
        * @warning  All modules must be build on the same compiler
        */
        template <typename Base, typename Plugin, typename... Args, typename = std::enable_if_t<std::is_base_of_v<IPlugin, Base>&& std::is_base_of_v<Base, Plugin>>>
        void CreatePlugin([[maybe_unused]] Args&&... args)
        {
            if(const auto pluginName = HF_CLASSNAME_RT(Base); m_Plugins.find(pluginName) == m_Plugins.end())
            {
                if(const auto pPlugin = HF_NEW Plugin(std::forward<Args>(args)...); !pPlugin) {
                    this->Shutdown(UTIL_FILE_LINE, fmt::format("Plugin: {} allocate memory failed!", pluginName));
                } else if(const auto [it, bResult] = m_Plugins.emplace(pluginName, pPlugin); !bResult) {
                    this->Shutdown(UTIL_FILE_LINE, fmt::format("Plugin: {} emplace object failed!", pluginName));
                } else {
                    it->second->m_pModuleManager = this;
                    UTIL_CONSOLE_INFO("Plugin: {} initialized!", pluginName);
                }
            } else {
                this->Shutdown(UTIL_FILE_LINE, fmt::format("Plugin: {} already has!", pluginName));
            }
        }

        /*
        * @brief    Get instance of plugin
        *
        * @tparam   Base    Type of abstract class inherited from IPlugin
        *
        * @return   Pointer on instance of Base plugin or nullptr if Base typename not found.
        * @warning  All modules must be build on the same compiler
        */
        template <typename Base, typename = std::enable_if_t<std::is_base_of_v<IPlugin, Base>>>
        [[nodiscard]] Base* GetPlugin() noexcept
        {
            auto& pPlugin = PluginPtr<Base>::m_pPlugin;
            if(!pPlugin) {
                const auto pluginName = HF_CLASSNAME_RT(Base);
                if(const auto it = m_Plugins.find(pluginName); it != m_Plugins.end()) {
                    pPlugin = static_cast<Base*>(it->second);
                } else {
                    this->Shutdown(UTIL_FILE_LINE, fmt::format("Plugin: {} not found!", pluginName));
                }
            }
            return pPlugin;
        }

        /*
        * @brief    Remove instance of plugin
        *
        * @tparam   Base    Type of abstract class inherited from IPlugin
        */
        template <typename Base, typename... Args,
            typename = std::enable_if_t<std::is_base_of_v<IPlugin, Base>>>
            void RemovePlugin() noexcept {
            m_Plugins.erase(HF_CLASSNAME_RT(Base));
        }

    public:
        /*
        * @brief    Get directories
        * @return   @code{.cpp} const Directories* @endcode
        */
        [[nodiscard]] const Directories* GetDirectories() const noexcept {
            return &m_Directories;
        }

        /*
        * @brief    Get Service name
        * @return   @code{.cpp} const std::string& @endcode
        */
        [[nodiscard]] const std::string& GetServiceName() const noexcept {
            return m_ServiceName;
        }

    private:
    #if HF_PLATFORM == HF_PLATFORM_WIN
        static BOOL WINAPI CtrlHandler(DWORD) {
            ModuleManager::m_pModuleManager.load()->Shutdown();
            return TRUE;
        }
    #elif HF_PLATFORM == HF_PLATFORM_LINUX
        static void SigHandler(int)
        {

        }
    #endif

        /**
        * @brief    Initialize framework
        * @param    moduleNames     List of module names for dynamic load
        */
        void Initialize(std::vector<std::string>& moduleNames)
        {
            UTIL_CONSOLE_INFO("Initialization ModuleManager...");

            // Initialize state and modules
            m_Modules.reserve(moduleNames.size());
            for(auto& name : moduleNames)
            {
                auto module = std::make_unique<Module>(std::move(name));

            #if HF_PLATFORM == HF_PLATFORM_WIN
                module->m_Name.append(".dll");
            #elif HF_PLATFORM == HF_PLATFORM_LINUX
                module->m_Name.append(".so");
            #else 
            #error Module extension not has for current platform!
            #endif

                if(module->m_pHandle = static_cast<HF_MODULE_HANDLE>(HF_MODULE_LOAD(module->m_Name.c_str())); !module->m_pHandle) {
                    this->Shutdown(UTIL_FILE_LINE, fmt::format("Module: {} load failed!", module->m_Name));
                    return;
                } else if(module->m_pMain = reinterpret_cast<Module::ModuleEP>(HF_MODULE_GETSYM(module->m_pHandle, HF_MODULE_CALLBACK)); !module->m_pMain) {
                    this->Shutdown(UTIL_FILE_LINE, fmt::format("Module: {} entry point not found!", module->m_Name));
                    return;
                } else {
                    UTIL_CONSOLE_INFO("Module: {} loaded!", module->m_Name);
                    module->m_pMain(this, EModuleState::Init);
                    m_Modules.emplace_back(std::move(module));
                }
            }

            // IPlugin->Initialize virtual call for Plugins
            for(const auto& plugin : m_Plugins) {
                if(m_bFinish || !plugin.second->Initialize()) {
                    return;
                }
            }

            // IPlugin->Config virtual call for Plugins
            for(const auto& plugin : m_Plugins) {
                if(m_bFinish || !plugin.second->Config()) {
                    return;
                }
            }

            // IPlugin->Execute virtual call for Plugins
            for(const auto& plugin : m_Plugins) {
                if(m_bFinish || !plugin.second->Execute()) {
                    return;
                }
            }

            // IPlugin->Update virtual call for Plugins
            for(;; std::this_thread::sleep_for(std::chrono::milliseconds(1))) {
                for(const auto& plugin : m_Plugins) {
                    if(m_bFinish || !plugin.second->Update()) {
                        return;
                    }
                }
            }
        }

        /* @brief Finalize framework */
        void Finalize()
        {
            // IPlugin->Finalize virtual call for Plugins
            for(const auto& plugin : m_Plugins) {
                (void)plugin.second->Finalize();
            }

            for(const auto& module : m_Modules) {
                module->m_pMain(this, EModuleState::Free);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if(!m_Plugins.empty()) {
                UTIL_CONSOLE_ERROR("Memory leak detected in one of the modules!");
                m_Plugins.clear();
            }

            for(const auto& module : m_Modules) {
                UTIL_CONSOLE_INFO("Module: {} unloaded!", module->m_Name);
                HF_MODULE_UNLOAD(module->m_pHandle);
            }

            if(!m_LastError.empty()) {
                UTIL_CONSOLE_ERROR("{}", m_LastError);
            }

            UTIL_CONSOLE_INFO("Finalize ModuleManager");
        }

    private:
        Directories m_Directories;
        std::unordered_map<std::string, IPlugin*> m_Plugins;
        std::vector<std::unique_ptr<Module>> m_Modules;
        std::string m_ServiceName;
        std::string m_LastError;
        bool m_bFinish;
        
        static inline std::atomic<ModuleManager*> m_pModuleManager = nullptr;
    };

    // HelenaFramework Main
    __forceinline void HelenaFramework(std::string& serviceName, std::string&& pathConfig, std::string&& pathModule, std::string&& pathResource, std::vector<std::string>& moduleNames)
    {
        if(pathConfig.back() != HF_SEPARATOR) {
            pathConfig += HF_SEPARATOR;
        }

        if(pathModule.back() != HF_SEPARATOR) {
            pathModule += HF_SEPARATOR;
        }

        if(pathResource.back() != HF_SEPARATOR) {
            pathResource += HF_SEPARATOR;
        }

        ModuleManager moduleManager(serviceName, pathConfig, pathModule, pathResource);

    #if HF_PLATFORM == HF_PLATFORM_WIN
        SetConsoleCtrlHandler(ModuleManager::CtrlHandler, TRUE);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        SetConsoleTitle(moduleManager.GetServiceName().c_str());
    #elif HF_PLATFORM == HF_PLATFORM_LINUX
        signal(SIGHUP,  SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTERM, ModuleManager::SigHandler);
        signal(SIGSTOP, ModuleManager::SigHandler);
        signal(SIGINT,  ModuleManager::SigHandler);
    #else
        #error Unknown platform
    #endif

        moduleManager.Initialize(moduleNames);
        moduleManager.Finalize();
    }
}

#endif // COMMON_MODULEMANAGER_HPP