#ifndef COMMON_MODULEMANAGER_IPP
#define COMMON_MODULEMANAGER_IPP

#include "IPlugin.hpp"
#include "Service.hpp"

namespace Helena
{
    template <typename Base, typename Plugin, typename... Args>
    void ModuleManager::CreatePlugin(const EPluginPriority priority, [[maybe_unused]] Args&&... args) {
        static_assert(std::is_base_of_v<IPlugin, Base> && std::is_base_of_v<Base, Plugin>);
        if(const auto pluginName = HF_CLASSNAME_RT(Base); m_Plugins.find(pluginName) == m_Plugins.end()) 
        {
            if(auto plugin = std::make_unique<Plugin>(std::forward<Args>(args)...); !plugin) {
                m_Service->Shutdown(UTIL_FILE_LINE, fmt::format("Plugin: {} allocate memory failed!", pluginName));
            } else if(const auto [it, bResult] = m_Plugins.emplace(pluginName, std::move(plugin)); !bResult) {
                m_Service->Shutdown(UTIL_FILE_LINE, fmt::format("Plugin: {} emplace object failed!", pluginName));
            } else {
                it->second->m_Service = m_Service;
                auto& plugin = m_PluginsPriority.emplace_back();
                plugin.m_Plugin = it->second.get();
                plugin.m_Priority = priority;
                UTIL_CONSOLE_INFO("Plugin: {} initialized!", pluginName);
            }
        } else {
            m_Service->Shutdown(UTIL_FILE_LINE, fmt::format("Plugin: {} already has!", pluginName));
        }
    }

    template <typename Base>
    [[nodiscard]] Base* ModuleManager::GetPlugin() noexcept {
        static_assert(std::is_base_of_v<IPlugin, Base>);
        auto& pPlugin = PluginCache<Base>::m_pPlugin;
        if(!pPlugin) {
            const auto pluginName = HF_CLASSNAME_RT(Base);
            if(const auto it = m_Plugins.find(pluginName); it != m_Plugins.end()) {
                pPlugin = static_cast<Base*>(it->second.get());
            } else {
                m_Service->Shutdown(UTIL_FILE_LINE, fmt::format("Plugin: {} not found!", pluginName));
            }
        }
        return pPlugin;
    }

    template <typename Base, typename... Args>
    void ModuleManager::RemovePlugin() noexcept {
        static_assert(std::is_base_of_v<IPlugin, Base>);
        if(const auto it = m_Plugins.find(HF_CLASSNAME_RT(Base)); it != m_Plugins.end()) {
            m_PluginsPriority.erase(std::remove_if(m_PluginsPriority.begin(), m_PluginsPriority.end(), [&plugin = it->second](const auto& pluginPriority) {
                return pluginPriority.m_Plugin == plugin.get();
            }));
            m_Plugins.erase(it);
        }
        PluginCache<Base>::m_pPlugin = nullptr;
    }

    [[nodiscard]] inline void ModuleManager::InitModules(Service* service, const std::string_view moduleNames) 
    {
        m_Service = service;

        const auto modules = Util::Split<std::string_view>(moduleNames);
        if(modules.empty()) {
            UTIL_CONSOLE_ERROR("Service: {} module list is empty!", service->GetName());
            return;
        }

        for(const auto name : modules) 
        {
            auto module = std::make_unique<Module>();
            if(!module) {
                UTIL_CONSOLE_ERROR("Module: {} allocate memory failed!", name);
                return;
            } 

        #if HF_PLATFORM == HF_PLATFORM_WIN
            std::string moduleFile = fmt::format("{}.dll", name);
        #elif HF_PLATFORM == HF_PLATFORM_LINUX
            std::string moduleFile = fmt::format("{}.so", name);
        #else 
            #error Module extension not has for current platform!
        #endif

            module->m_Name = name;
            if(module->m_Handle = static_cast<HF_MODULE_HANDLE>(HF_MODULE_LOAD(moduleFile.c_str())); !module->m_Handle) {
                UTIL_CONSOLE_ERROR("Module: {} load failed!", name);
                return;
            }

            if(module->m_EntryPoint = reinterpret_cast<EntryPoint>(HF_MODULE_GETSYM(module->m_Handle, HF_MODULE_CALLBACK)); !module->m_EntryPoint) {
                UTIL_CONSOLE_ERROR("Module: {} entry point not found!", name);
                return;
            }

            UTIL_CONSOLE_INFO("Module: {} loaded!", name);
            module->m_EntryPoint(this, EModuleState::Init);
            m_Modules.emplace_back(std::move(module));
        }

        std::sort(m_PluginsPriority.begin(), m_PluginsPriority.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.m_Priority > rhs.m_Priority;
        });

    }

    [[nodiscard]] inline void ModuleManager::FreeModules()
    {
        for(const auto& plugin : m_PluginsPriority) {
            (void)plugin.m_Plugin->Finalize();
        }

        for(const auto& module : m_Modules) {
            module->m_EntryPoint(this, EModuleState::Free);
        }

        Util::Sleep(1000);

        if(!m_Plugins.empty()) {
            UTIL_CONSOLE_ERROR("Memory leak detected in one of the modules!");
            m_Plugins.clear();
            m_PluginsPriority.clear();
        }

        for(const auto& module : m_Modules) {
            UTIL_CONSOLE_INFO("Module: {} unloaded!", module->m_Name);
            HF_MODULE_UNLOAD(module->m_Handle);
        }

        m_Modules.clear();
        UTIL_CONSOLE_INFO("Finalize ModuleManager");
    }

    [[nodiscard]] inline const std::vector<ModuleManager::PluginPriority>& ModuleManager::GetPlugins() const noexcept {
        return m_PluginsPriority;
    }
}

#endif // COMMON_MODULEMANAGER_IPP