#ifndef COMMON_MODULEMANAGER_HPP
#define COMMON_MODULEMANAGER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <thread>
#include <type_traits>

#include "IPlugin.hpp"
#include "Platform.hpp"
#include "Util.hpp"
#include "Format.hpp"

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
            inline static Type* m_pPlugin {nullptr};
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
            , m_bFinish(false) {}

        ~ModuleManager() = default;
        ModuleManager(const ModuleManager&) = delete;
        ModuleManager(ModuleManager&&) noexcept = delete;
        ModuleManager& operator=(const ModuleManager&) = delete;
        ModuleManager& operator=(ModuleManager&&) noexcept = delete;

    public:
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
            if(!m_bFinish) {
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
        *           then the framework will finalize with an error identifier.
        */
        template <typename Base, typename Plugin, typename... Args, 
            typename = std::enable_if_t<std::is_base_of_v<IPlugin, Base> && 
                std::is_base_of_v<Base, Plugin>>>
        void CreatePlugin([[maybe_unused]] Args&&... args) 
        {
            if(const auto pluginName = HF_CLASSNAME_RT(Base); m_Plugins.find(pluginName) == m_Plugins.end())
            {
                if(const auto pPlugin = HF_NEW Plugin(std::forward<Args>(args)...); !pPlugin) {
                    Shutdown(Util::GetFileName(__FILE__), __LINE__, 
                        fmt::format("Plugin: {} allocate memory failed!", pluginName));
                } else if(const auto [it, bResult] = m_Plugins.emplace(pluginName, pPlugin); !bResult) {
                    Shutdown(Util::GetFileName(__FILE__), __LINE__, 
                        fmt::format("Plugin: {} emplace object failed!", pluginName));
                } else {
                    it->second->m_pModuleManager = this;
                    std::cerr << "[Info ] Plugin: " << pluginName << " initialized!" << std::endl;
                }
            } else {
                Shutdown(Util::GetFileName(__FILE__), __LINE__, 
                    fmt::format("Plugin: {} already has!", pluginName));
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
                    Shutdown(HF_FILE_LINE, fmt::format("Plugin: {} not found!", pluginName));
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
        /**
        * @brief    Initialize framework
        * @param    moduleNames     List of module names for dynamic load
        */
        void Initialize(std::vector<std::string>& moduleNames)
        {
            std::cerr << "[Info ] Intitialization ModuleManger..." << std::endl;
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
                    Shutdown(Util::GetFileName(__FILE__), __LINE__, fmt::format("Module: {} load failed!", module->m_Name));
                    return;
                } else if(module->m_pMain = reinterpret_cast<Module::ModuleEP>(HF_MODULE_GETSYM(module->m_pHandle, HF_MODULE_CALLBACK)); !module->m_pMain) {
                    Shutdown(Util::GetFileName(__FILE__), __LINE__, fmt::format("Module: {} entry point not found!", module->m_Name));
                    return;
                } else {
                    std::cerr << "[Info ] Module: " << module->m_Name << " loaded!" << std::endl;
                    module->m_pMain(this, EModuleState::Init);
                    m_Modules.emplace_back(std::move(module));
                }
            }

            // IPlugin->Initialize virtual call for Plugins
            for(const auto& plugin : m_Plugins) {
                if(m_bFinish || !plugin.second->Initialize()) {
                    break;
                }
            }
        }

        void Config() const {
            // IPlugin->Config virtual call for Plugins
            for(const auto& plugin : m_Plugins) {
                if(m_bFinish || !plugin.second->Config()) {
                    break;
                }
            }
        }

        void Execute() const {
            // IPlugin->Execute virtual call for Plugins
            for(const auto& plugin : m_Plugins) {
                if(m_bFinish || !plugin.second->Execute()) {
                    break;
                }
            }
        }

        void Update() const 
        {
            if(!m_bFinish) 
            {
                for(;; std::this_thread::sleep_for(std::chrono::milliseconds(1))) {
                    // IPlugin->Update virtual call for Plugins
                    for(const auto& plugin : m_Plugins) {
                        if(m_bFinish || !plugin.second->Update()) {
                            return;
                        }
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

            if(!m_Plugins.empty()) {
                std::cerr 
                    << "[Error] A memory leak was detected in one of the modules!" 
                    << std::endl;
                m_Plugins.clear();
            }

            for(const auto& module : m_Modules) {
                std::cerr 
                    << "[Info ] Module: " 
                    << module->m_Name 
                    << " unloaded!" 
                    << std::endl;
                HF_MODULE_UNLOAD(module->m_pHandle);
            }

            if(m_bFinish) {
                std::cerr << m_LastError << std::endl;
            }

            std::cerr << "[Info ] Finalize ModuleManager" << std::endl;
        }

    private:
        Directories m_Directories;
        std::unordered_map<std::string, IPlugin*> m_Plugins;
        std::vector<std::unique_ptr<Module>> m_Modules;
        std::string m_ServiceName;
        std::string m_LastError;
        bool m_bFinish;
    };

    // HelenaFramework Main
    __forceinline void HelenaFramework(std::string& serviceName, std::string&& configPath, std::string&& modulePath, std::string&& resourcePath, std::vector<std::string>& moduleNames)
    {
        if(configPath.back() != HF_SEPARATOR) {
            configPath += HF_SEPARATOR;
        }

        if(modulePath.back() != HF_SEPARATOR) {
            modulePath += HF_SEPARATOR;
        }

        if(resourcePath.back() != HF_SEPARATOR) {
            resourcePath += HF_SEPARATOR;
        }

        ModuleManager moduleManager(serviceName, configPath, modulePath, resourcePath);

    #if HF_PLATFORM == HF_PLATFORM_WIN
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        SetConsoleTitle(serviceName.c_str());

        if(const auto pHandle = GetConsoleWindow(); pHandle) {
            if(const auto pMenu = GetSystemMenu(pHandle, FALSE); pMenu) {
                EnableMenuItem(pMenu, SC_CLOSE, MF_DISABLED | MF_BYCOMMAND);
            }
        }

    #elif HF_PLATFORM == HF_PLATFORM_LINUX
        
    #else
        #error Unknown platform
    #endif

        moduleManager.Initialize(moduleNames);
        moduleManager.Config();
        moduleManager.Execute();
        moduleManager.Update();
        moduleManager.Finalize();
    }
}

#endif // COMMON_MODULEMANAGER_HPP