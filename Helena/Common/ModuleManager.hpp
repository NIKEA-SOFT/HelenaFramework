#ifndef COMMON_MODULEMANAGER_HPP
#define COMMON_MODULEMANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>

#include "Util.hpp"

namespace Helena
{
    enum class EModuleState : uint8_t {
        Init,
        Free
    };

    enum class EPluginPriority : uint8_t {
        LOW,
        NORMAL,
        HIGH
    };
    
    
    class Service;
    class IPlugin;
    class ModuleManager final
    {
        friend class Service;
        
        using EntryPoint = void (*)(ModuleManager*, EModuleState);

        template <typename Type>
        struct PluginCache { 
            inline static Type* m_pPlugin{nullptr}; 
        };

        struct PluginPriority {
            IPlugin* m_Plugin;
            EPluginPriority m_Priority;
        };

        struct Module {
            explicit Module() : m_Handle(nullptr)
                              , m_EntryPoint(nullptr) {}

            std::string_view    m_Name;
            HF_MODULE_HANDLE    m_Handle;
            EntryPoint          m_EntryPoint;
        };

    public:
        explicit ModuleManager() : m_Service(nullptr) {}
        ~ModuleManager() = default;
        ModuleManager(const ModuleManager&) = delete;
        ModuleManager(ModuleManager&&) noexcept = delete;
        ModuleManager& operator=(const ModuleManager&) = delete;
        ModuleManager& operator=(ModuleManager&&) noexcept = delete;

    public:
        /**
        * @brief    Create instance of plugin into the map
        *
        * @tparam   Base    Type of abstract class inherited from IPlugin
        * @tparam   Plugin  Type of plugin class inherited from Base
        * 
        * @param    args    Arguments for ctor of Plugin
        *
        * @note     If it was not possible to create an instance of the class,
        *           then the framework will finalize with error
        * @warning  All modules must be build on the same compiler
        */
        template <typename Base, typename Plugin, typename... Args>
        void CreatePlugin(const EPluginPriority priority = EPluginPriority::NORMAL, [[maybe_unused]] Args&&... args);

        /*
        * @brief    Get instance of plugin
        *
        * @tparam   Base    Type of abstract class inherited from IPlugin
        *
        * @return   Pointer on instance of Base plugin or nullptr if Base typename not found.
        * @warning  All modules must be build on the same compiler
        */
        template <typename Base>
        [[nodiscard]] Base* GetPlugin() noexcept;

        /*
        * @brief    Remove instance of plugin
        *
        * @tparam   Base    Type of abstract class inherited from IPlugin
        */
        template <typename Base, typename... Args>
        void RemovePlugin() noexcept;

    private:
        void InitModules(Service* service, const std::string_view moduleNames);
        void FreeModules();

    private:
        [[nodiscard]] const std::vector<ModuleManager::PluginPriority>& GetPlugins() const noexcept;

    private:
        std::unordered_map<std::string, std::unique_ptr<IPlugin>> m_Plugins;
        std::vector<PluginPriority> m_PluginsPriority;
        std::vector<std::unique_ptr<Module>> m_Modules;
        Service* m_Service;
    };
}

#include "ModuleManager.ipp"

#endif // COMMON_MODULEMANAGER_HPP