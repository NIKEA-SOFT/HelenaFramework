#include "Module.hpp"
#include "Plugins/PluginTestA.hpp"

#include <Helena/Common/HFApp.hpp>
#include <Helena/Module/ModuleTestB/Module.hpp>

namespace Helena
{
    bool ModuleTestA::AppInit() 
    {
        std::cout << "AppInit: " << HF_CLASSNAME(ModuleTestA) << std::endl;
        std::cout << "Try get pointer on " << HF_CLASSNAME(ModuleTestB) << std::endl;

        // Get third party module pointer 
        if(this->m_pModuleTestB = this->GetApp()->GetModule<ModuleTestB>(); this->m_pModuleTestB) {
            std::cout << "Success get pointer on " << HF_CLASSNAME(ModuleTestB) << std::endl;
        } else std::cout << "Failure get pointer on " << HF_CLASSNAME(ModuleTestB) << std::endl;

        // Create and add plugin in this module
        if(const auto pPlugin = this->AddPlugin<PluginTestA>(); pPlugin) {
            std::cout << "Plugin work, value: " << pPlugin->m_Value << std::endl;
        }
        
        /* Examples how get plugin from this module */

        // Very good!
        // It's cheap if you're using GetPlugin.
        // GetPlugin is built on type indexing and is therefore performant
        if(const auto pPlugin = this->GetPlugin<PluginTestA>(); pPlugin) {
            std::cout << "Plugin work, value: " << pPlugin->m_Value << std::endl;
        }

        // Good!
        // This does the same as GetPlugin, however, 
        // this method is less efficient than GetPlugin, 
        // therefore, if you use the plugin more than once, 
        // then you better create the pointer on Plugin in your class 
        // so as not to lose performance while performing searches using this method all time.
        // This method is mainly intended for third-party modules to get pointers to your plugins, 
        // they cannot do this through GetPlugin, consider it the pay of performance
        if(const auto pPlugin = this->GetPluginByName<PluginTestA>(); pPlugin) {
            std::cout << "Plugin work, value: " << pPlugin->m_Value << std::endl;
        }

        // NOTE: Here's a quick rundown for "GetPlugin" and "GetPluginByName"

        // If you want to use plugins in this module (from this module), 
        // then prefer GetPlugin without the need to store pointers in the class, 
        // or use GetPluginByName, but in this case it will be more efficient to 
        // get the result once and store it as a pointer inside your class.

        // If you want to use plugins from third party module, forgot about GetPlugin!
        // Use GetPluginByName exclusively, otherwise you will shoot yourself in the foot
        return true;
    }

    bool ModuleTestA::AppConfig() 
    {
        // Remove plugin from this module
        this->RemovePlugin<PluginTestA>();
        std::cout << "AppConfig: " << HF_CLASSNAME(ModuleTestA) << std::endl;
        return true;
    }

    bool ModuleTestA::AppStart() 
    {
        std::cout << "AppStart: " << HF_CLASSNAME(ModuleTestA) << std::endl;
        return true;
    }

    bool ModuleTestA::AppUpdate() 
    {
        //std::cout << "AppUpdate: " << HF_CLASSNAME(ModuleTestA) << std::endl;
        return true;
    }

    bool ModuleTestA::AppShut() 
    {
        std::cout << "AppShut: " << HF_CLASSNAME(ModuleTestA) << std::endl;
        return true;
    }
}