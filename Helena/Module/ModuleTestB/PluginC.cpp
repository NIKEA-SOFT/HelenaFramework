#include "PluginC.hpp"

#include "Module.hpp"

namespace Helena
{
    void PluginC::Zoo() 
    {
        auto myModule = this->GetModule();
        std::cout << "PluginC Module: " << myModule << std::endl;
    }
}