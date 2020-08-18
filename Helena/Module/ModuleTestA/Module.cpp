#include "Module.hpp"
#include "PluginA.hpp"
#include "PluginB.hpp"

#include <Helena/Common/HFApp.hpp>
#include <Helena/Module/ModuleTestB/Module.hpp>
#include <Helena/Module/ModuleTestB/PluginC.hpp>

namespace Helena
{
    bool ModuleTestA::AppInit() 
    {
        // Get pointer on third module class instance
        this->m_pModuleTestB = this->GetApp()->GetModule<ModuleTestB>();
        std::cout << "Get module: " << HF_CLASSNAME(ModuleTestB) << (this->m_pModuleTestB ? " success" : " failure") << std::endl;
        
        this->m_pModuleTestB->Hello();
        
        // Get plugin from third module
        /*
        this->m_pModuleTestB->GetPluginC()->Zoo();
        if(const auto Plugin = this->m_pModuleTestB->GetPluginC(); Plugin) {
            std::cout << "Get module plugin: " << HF_CLASSNAME(PluginC) << " success" << std::endl;
            Plugin->Zoo();
        } else std::cout << "Get module plugin: " << HF_CLASSNAME(PluginC) << " failure" << std::endl;
        */

        // Get plugin from this module
        if(const auto Plugin = this->AddPlugin<PluginA>(); Plugin) {
            std::cout << "Get plugin: " << HF_CLASSNAME(PluginA) << " success, call: " << HF_CLASSNAME(Plugin->Foo()) << std::endl;
            Plugin->Foo();
            std::cout << "Remove plugin: " << HF_CLASSNAME(PluginA) << " from Module class" << std::endl;
            this->RemovePlugin<PluginA>();
        } else std::cout << "Get plugin: " << HF_CLASSNAME(PluginA) << " failure" << std::endl;

        // Get plugin from this module
        if(const auto Plugin = this->AddPlugin<PluginB>(); Plugin) {
            std::cout << "Get plugin: " << HF_CLASSNAME(PluginB) << " success, call: " << HF_CLASSNAME(Plugin->Boo()) << std::endl;
            Plugin->Boo();
            std::cout << "Remove plugin: " << HF_CLASSNAME(PluginB) << " from Module class" << std::endl;
            this->RemovePlugin<PluginB>();
        } else std::cout << "Get plugin: " << HF_CLASSNAME(PluginB) << " failure" << std::endl;

        std::cout << "AppInit: " << HF_CLASSNAME(ModuleTestA) << std::endl;
        return true;
    }

    bool ModuleTestA::AppConfig() 
    {
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