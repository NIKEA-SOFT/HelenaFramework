#include "Module.hpp"
#include "PluginC.hpp"

#include <Helena/Module/ModuleTestA/Module.hpp>
#include <Helena/Module/ModuleTestA/PluginA.hpp>

#include <Helena/Common/HFApp.hpp>

namespace Helena
{
    void ModuleTestB::Hello() 
    {
        std::cout << "Hello from ModuleB" << std::endl;
    }

    bool ModuleTestB::AppInit() 
    {
        this->m_pModuleTestA = this->GetApp()->GetModule<ModuleTestA>();
        std::cout << "Get module: " << HF_CLASSNAME(ModuleTestA) << (this->m_pModuleTestA ? " success" : " failure") << std::endl;

        std::cout << "AppInit: " << HF_CLASSNAME(ModuleTestB) << std::endl;
        return true;
    }

    bool ModuleTestB::AppConfig() 
    {
        std::cout << "AppConfig: " << HF_CLASSNAME(ModuleTestB) << std::endl;
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

    PluginC* ModuleTestB::GetPluginC() const {
        return this->GetPlugin<PluginC>();
    }
}