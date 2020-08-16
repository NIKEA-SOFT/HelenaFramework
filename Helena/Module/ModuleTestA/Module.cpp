#include "Module.hpp"
#include "Plugins/PluginTestA.hpp"

#include <Helena/Common/HFApp.hpp>
#include <Helena/Module/ModuleTestB/Module.hpp>
#include <Helena/Module/ModuleLog/Module.hpp>

namespace Helena
{
    bool ModuleTestA::AppInit() 
    {
        std::cout << "AppInit: " << HF_CLASSNAME(ModuleTestA) << std::endl;
        std::cout << "Try get pointer on " << HF_CLASSNAME(ModuleTestB) << std::endl;

        if(this->m_pModuleLog = this->GetApp()->GetModule<ModuleLog>(); this->m_pModuleLog) {
            this->m_pModuleLog->Log("Hello World");
        }

        
    	
        // Get third party module pointer 
        if(this->m_pModuleTestB = this->GetApp()->GetModule<ModuleTestB>(); this->m_pModuleTestB) {
            std::cout << "Success get pointer on " << HF_CLASSNAME(ModuleTestB) << std::endl;
        	
        } else std::cout << "Failure get pointer on " << HF_CLASSNAME(ModuleTestB) << std::endl;

        // Create and add plugin in this module
        if(const auto pPlugin = this->AddPlugin<PluginTestA>(); pPlugin) {
        	pPlugin->GetModule()->GetPlugin<PluginTestA>();
            std::cout << "Plugin work, value: " << pPlugin->m_Value << std::endl;
        }
        
        /* Examples how get plugin from this module */

        // Very good!
        // It's cheap if you're using GetPlugin.
        // GetPlugin is built on type indexing and is therefore performant
        if(const auto pPlugin = this->GetPlugin<PluginTestA>(); pPlugin) {
            std::cout << "Plugin work, value: " << pPlugin->m_Value << std::endl;
        }

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
	
    ModuleTestB* ModuleTestA::GetModuleTestB() const {
        return nullptr;
    }
}