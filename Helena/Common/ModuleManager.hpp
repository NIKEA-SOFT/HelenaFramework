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

#include "Platform.hpp"
#include "IPlugin.hpp"

namespace Helena
{
    // State of module for EntryPoint
    enum class EModuleState : uint8_t {
        Init,
        Free
    };

    class IPlugin;
    class ModuleManager final 
    {
        friend void HelenaFramework(std::string, std::string, std::string, std::string, std::vector<std::string>);

        // State of manager
        enum class EManagerError {
            NoError             = 0,        // No error
            ModuleLoad          = 1,        // Module load fail (file not exist or access failure)
            ModuleEP            = 2,        // Module success load but EntryPoint function not found
            PluginCreateAlready = 3,        // Plugin with this type is already registered in one of the modules
            PluginCreateAlloc   = 4,        // Plugin create for this type failed, allocate memory failure
            PluginCreateEmplace = 5,        // Plugin create for this type failed, allocate memory for emplace in hash map failure
            PluginGet           = 6,        // Plugin get for this type failed, plugin not exist in hash map

            SizeOfErrors
        };

        // Storage directories
        class Directories final 
        {
        public:            
            Directories(std::string configPath, std::string modulePath, std::string resourcePath) 
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
            * 
            * @return   @code{.cpp} const std::string& @endcode
            */
            [[nodiscard]] const std::string& GetConfigPath() const noexcept {
                return m_ConfigPath;
            }

            /**
            * @brief    Get the path to modules
            *
            * @return   @code{.cpp} const std::string& @endcode
            */
            [[nodiscard]] const std::string& GetModulePath() const noexcept {
                return m_ModulePath;
            }

            /**
            * @brief    Get the path to resources (configs/resource of services)
            *
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
        * @param    appName         -> Application name taken from the service file name
        * @param    configPath      -> Path to config files of modules
        * @param    modulePath      -> Path to files of modules (dll/so)
        * @param    resourcePath    -> Path to resources of service
        */
        explicit ModuleManager(std::string appName, std::string configPath, 
            std::string modulePath, std::string resourcePath)
            : m_Directories(std::move(configPath), std::move(modulePath), std::move(resourcePath))
            , m_AppName(std::move(appName))
            , m_Error(EManagerError::NoError)
            , m_bInitialized(false)
            , m_bUpdate(false) {}

        ~ModuleManager() = default;
        ModuleManager(const ModuleManager&) = delete;
        ModuleManager(ModuleManager&&) noexcept = delete;
        ModuleManager& operator=(const ModuleManager&) = delete;
        ModuleManager& operator=(ModuleManager&&) noexcept = delete;

    private:
        /**
        * @brief    Initialize framework
        * 
        * @param    moduleNames     -> List of module names for dynamic load
        */
        void Initialize(std::vector<std::string>& moduleNames)
        {
            if(m_bInitialized) {
                std::cerr << "[Error] ModuleManager already initialized!" << std::endl;
                return;
            }

            std::cout << "[Info ] Intitialization ModuleManger..." << std::endl;
            // Initialize state and modules
            m_bInitialized = true;
            m_Modules.reserve(moduleNames.size());
            for(auto& name : moduleNames)
            {
                auto module = std::make_unique<Module>(std::move(name));

            #ifdef HF_PLATFORM_WIN
                module->m_Name.append(".dll");
            #elif HF_PLATFORM_LINUX
                module->m_Name.append(".so");
            #else 
                #error Module extension not has for current platform!
            #endif

                module->m_pHandle = static_cast<HF_MODULE_HANDLE>(HF_MODULE_LOAD(module->m_Name.c_str()));
                if(!module->m_pHandle) {
                    std::cerr << "[Error] Module: " << module->m_Name << ", load failure!" << std::endl;
                    m_Error = EManagerError::ModuleLoad;
                    return;
                }

                module->m_pMain = reinterpret_cast<Module::ModuleEP>(HF_MODULE_GETSYM(module->m_pHandle, HF_MODULE_CALLBACK));
                if(!module->m_pMain) {
                    std::cerr << "[Error] Module: " << module->m_Name << ", EP not found!" << std::endl;
                    m_Error = EManagerError::ModuleEP;
                    return;
                }
                std::cout << "[Info ] Module: " << module->m_Name << " loaded!" << std::endl;

                module->m_pMain(this, EModuleState::Init);
                m_Modules.emplace_back(std::move(module));
            }

            
            // IPlugin->Initialize virtual call for Plugins
            for(const auto& plugin : m_Plugins) 
            {
                if(!plugin.second->Initialize() || m_Error != EManagerError::NoError) {
                    return;
                }
            }

            // IPlugin->Config virtual call for Plugins
            for(const auto& plugin : m_Plugins) 
            {
                if(!plugin.second->Config() || m_Error != EManagerError::NoError) {
                    return;
                }
            }

            // IPlugin->Execute virtual call for Plugins
            for(const auto& plugin : m_Plugins) 
            {
                if(!plugin.second->Execute() || m_Error != EManagerError::NoError) {
                    return;
                }
            }

            
            // IPlugin->Update virtual call for Plugins
            for(;; std::this_thread::sleep_for(std::chrono::milliseconds(1)))
            {
                for(const auto& plugin : m_Plugins)
                {
                    if(!plugin.second->Update() || m_Error != EManagerError::NoError) {
                        return;
                    }
                }
            }

            return;
        }

        /* @brief Finalize framework */
        void Finalize() 
        {
            if(!m_bInitialized) {
                return;
            }

            // IPlugin->Finalize virtual call for Plugins
            for(const auto& plugin : m_Plugins) {
                (void)plugin.second->Finalize();
            }

            for(const auto& module : m_Modules) {
                module->m_pMain(this, EModuleState::Free);
            }

            if(!m_Plugins.empty()) {
                std::cerr << "[Error] A memory leak was detected in one of the modules!" << std::endl;
                m_Plugins.clear();
            }

            for(const auto& module : m_Modules) {
                std::cout << "[Info ] Module: " << module->m_Name << " unloaded!" << std::endl;
                HF_MODULE_UNLOAD(module->m_pHandle);
            }

            std::cout << "[Info ] Finalize ModuleManager";
            if(m_Error != EManagerError::NoError) {
                std::cout << " with error: " << std::underlying_type<EManagerError>::type(m_Error);
            }
            std::cout << std::endl;
        }

    public:
        /**
        * @brief    Create instance of plugin into the map
        * 
        * @tparam   Base    -> Type of abstract class inherited from IPlugin
        * @tparam   Plugin  -> Type of plugin class inherited from Base 
        * @param    args    -> Arguments for ctor of Plugin
        * 
        * @note     If it was not possible to create an instance of the class, 
        *           then the framework will finalize with an error identifier.
        */
        template <typename Base, typename Plugin, typename... Args, 
            typename = std::enable_if_t<std::is_base_of_v<IPlugin, Base> && std::is_base_of_v<Base, Plugin>>>
        void CreatePlugin([[maybe_unused]] Args&&... args)
        {
            const auto pluginName = HF_CLASSNAME_RT(Base);
            if(const auto it = m_Plugins.find(pluginName); it != m_Plugins.end()) {
                std::cerr << "[Error]\tPlugin: " << pluginName << " already has!" << std::endl;
                m_Error = EManagerError::PluginCreateAlready;
                return;
            }

            if(const auto pPlugin = HF_NEW Plugin(std::forward<Args>(args)...); !pPlugin) {
                std::cerr << "[Error] Plugin: " << pluginName << " allocate failure!" << std::endl;
                m_Error = EManagerError::PluginCreateAlloc;
            } else if(const auto [it, bResult] = m_Plugins.emplace(pluginName, pPlugin); !bResult) {
                std::cerr << "[Error] Plugin: " << pluginName << " emplace failure!" << std::endl;
                m_Error = EManagerError::PluginCreateEmplace;
            } else {
                std::cout << "[Info ] Plugin: " << pluginName << " initialized!" << std::endl;
            }
        }

        /*
        * @brief    Get instance of plugin
        * 
        * @tparam   Base    -> Type of abstract class inherited from IPlugin
        * @return   Pointer on instance of Base plugin or nullptr if Base typename not found.
        * 
        * @warning  All modules must be build on the same compiler
        */
        template <typename Base, typename = std::enable_if_t<std::is_base_of_v<IPlugin, Base>>>
        [[nodiscard]] Base* GetPlugin()
        {
            auto& pPlugin = PluginPtr<Base>::m_pPlugin;
            if(!pPlugin) {
                const auto pluginName = HF_CLASSNAME_RT(Base);
                const auto it = m_Plugins.find(pluginName);
                if(it != m_Plugins.end()) {
                    pPlugin = static_cast<Base*>(it->second);
                } else { 
                    std::cerr << "[Error] Plugin: " << pluginName << " not found!" << std::endl;
                    m_Error = EManagerError::PluginGet;
                }
            }
            return pPlugin;
        }

        /*
        * @brief    Remove instance of plugin
        *
        * @tparam   Base    -> Type of abstract class inherited from IPlugin
        */
        template <typename Base, typename... Args, typename = std::enable_if_t<std::is_base_of_v<IPlugin, Base>>>
        void RemovePlugin()
        {
            if(const auto it = m_Plugins.find(HF_CLASSNAME_RT(Base)); it != m_Plugins.end()) {
                m_Plugins.erase(it);
            }
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
        * @brief    Get Application name
        * @return   @code{.cpp} const std::string& @endcode
        */
        [[nodiscard]] const std::string& GetAppName() const noexcept {
            return m_AppName;
        }


    private:
        Directories m_Directories;
        std::unordered_map<std::string, IPlugin*> m_Plugins;
        std::vector<std::unique_ptr<Module>> m_Modules;
        std::string m_AppName;
        EManagerError m_Error;
        bool m_bInitialized;
        bool m_bUpdate;
    };

    // HelenaFramework Main
    inline void HelenaFramework(std::string appName, std::string configPath, std::string modulePath, std::string resourcePath, std::vector<std::string> moduleNames) {
        ModuleManager moduleManager(std::move(appName), std::move(configPath), std::move(modulePath), std::move(resourcePath));
        moduleManager.Initialize(std::move(moduleNames));
        moduleManager.Finalize();
    }
}

#endif // COMMON_MODULEMANAGER_HPP