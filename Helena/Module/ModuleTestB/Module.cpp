#include "Module.hpp"

#include <Helena/Common/HFApp.hpp>
#include <Helena/Module/ModuleTestA/Module.hpp>

namespace Helena
{
    bool ModuleTestB::AppInit() 
    {
        std::cout << "AppInit: " << HF_CLASSNAME(ModuleTestB) << std::endl;

        // Try get pointer on ModuleTestA (third party module)
        if(this->m_pModuleTestA = this->GetApp()->GetModule<ModuleTestA>(); this->m_pModuleTestA) {
            std::cout << "Success get pointer on " << HF_CLASSNAME(ModuleTestB) << std::endl;
        	
        } else std::cout << "Failure get pointer on " << HF_CLASSNAME(ModuleTestB) << std::endl;

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
}