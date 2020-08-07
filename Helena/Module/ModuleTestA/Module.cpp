#include "Module.hpp"

namespace Helena
{
    bool ModuleTestA::AppInit() 
    {
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
        std::cout << "AppUpdate: " << HF_CLASSNAME(ModuleTestA) << std::endl;
        return true;
    }

    bool ModuleTestA::AppShut() 
    {
        std::cout << "AppShut: " << HF_CLASSNAME(ModuleTestA) << std::endl;
        return true;
    }
}