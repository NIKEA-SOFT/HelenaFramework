#include "PluginB.hpp"

#include "Module.hpp"

namespace Helena
{
    void PluginB::Boo()
    {
        auto myModule = this->GetModule();
        
        std::cout << "PluginB Module: " << myModule << std::endl;
    }
}