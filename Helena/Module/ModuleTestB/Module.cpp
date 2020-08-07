#include "Module.hpp"

namespace Helena
{
    bool ModuleTestB::AppInit() 
    {
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
        std::cout << "AppUpdate: " << HF_CLASSNAME(ModuleTestB) << std::endl;
        return true;
    }

    bool ModuleTestB::AppShut() 
    {
        std::cout << "AppShut: " << HF_CLASSNAME(ModuleTestB) << std::endl;
        return true;
    }
}