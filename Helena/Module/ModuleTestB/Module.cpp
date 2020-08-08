#include "Module.hpp"

#include <Helena/Common/HFApp.hpp>
#include <Helena/Module/ModuleTestA/Module.hpp>
#include <Helena/Module/ModuleTestA/Plugins/PluginTestA.hpp>

namespace Helena
{
    bool ModuleTestB::AppInit() 
    {
        std::cout << "AppInit: " << HF_CLASSNAME(ModuleTestB) << std::endl;

        // Try get pointer on ModuleTestA (third party module)
        if(this->m_pModuleTestA = this->GetApp()->GetModule<ModuleTestA>(); this->m_pModuleTestA) {
            std::cout << "Success get pointer on " << HF_CLASSNAME(ModuleTestB) << std::endl;
        } else std::cout << "Failure get pointer on " << HF_CLASSNAME(ModuleTestB) << std::endl;

        /* Example how getting plugins from third party module */
        
        // Good! It's return plugin from third party module, 
        // but it will cost complexity O(1) for unordered_map.
        // For standard C++20 has support heterogeneous lookup.
        if(const auto pPlugin = this->m_pModuleTestA->GetPluginByName<PluginTestA>(); pPlugin) {
            std::cout << "Plugin remote work, value: " << pPlugin->m_Value << std::endl;
        } else std::cout << "Plugin remote by index is nullptr" << std::endl;

        // Bad! Even though pPlugin is not nullptr and you get the expected result but it's UB
        // You should never use GetPlugin when using pointer on third party module.
        // GetPlugin was created solely to get its own plugins for the current module.
        // This gives good performance without having to store the plugin pointer anywhere. 
        // But remember: never call GetPlugin if you use a pointer to a third party module. 
        // It may seem to you that it works, but it does not, 
        // it works exactly as long as you are lucky with the coincidence of type index, 
        // but in 99% of cases you will get UB
        if(const auto pPlugin = this->m_pModuleTestA->GetPlugin<PluginTestA>(); pPlugin) {
            std::cout << "Plugin remote work, value: " << pPlugin->m_Value << std::endl;
        } else std::cout << "Plugin remote by index is nullptr" << std::endl;

        return true;
    }

    bool ModuleTestB::AppConfig() 
    {
        std::cout << "AppConfig: " << HF_CLASSNAME(ModuleTestB) << std::endl;

        // pPlugin is nullptr, i free this plugin in ModuleTestA
        // for show example, how plugin can be removed in runtime.
        // Pls, see AppConfig method in ModuleTestA for details.
        if(const auto pPlugin = this->m_pModuleTestA->GetPluginByName<PluginTestA>(); !pPlugin) {
            std::cout << "Plugin remote is nullptr" << std::endl;
        }

        return true;
    }

    bool ModuleTestB::AppStart() 
    {
        std::cout << "AppStart: " << HF_CLASSNAME(ModuleTestB) << std::endl;
        return true;
    }

    bool ModuleTestB::AppUpdate() 
    {
        //std::cout << "AppUpdate: " << HF_CLASSNAME(ModuleTestB) << std::endl;
        return true;
    }

    bool ModuleTestB::AppShut() 
    {
        std::cout << "AppShut: " << HF_CLASSNAME(ModuleTestB) << std::endl;
        return true;
    }
}