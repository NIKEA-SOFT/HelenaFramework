#include "Module.hpp"

#include "MyPlugin.hpp"

namespace Helena
{
    bool ModuleTestA::AppInit() 
    {
        if(auto myPlugin = this->AddPlugin<MyPlugin>(); myPlugin) {
            std::cout << "Get ptr on Plugin from Module class success!" << std::endl;
            std::cout << "Call plugin method: Foo" << std::endl;
            myPlugin->Foo();

            std::cout << "Remove MyPlugin from Module class" << std::endl;
            this->RemovePlugin<MyPlugin>();
        }


        std::cout << "AppInit: " << HF_CLASSNAME(ModuleTestA) << std::endl;
        std::cout << "Try get pointer on " << HF_CLASSNAME(ModuleTestB) << std::endl;

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