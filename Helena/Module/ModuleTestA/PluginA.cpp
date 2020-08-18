#include "PluginA.hpp"

#include "Module.hpp"

namespace Helena
{
    void PluginA::Foo() 
    {
        auto myModule = this->GetModule();
        
        std::cout << "PluginA Module: " << myModule << std::endl;
    }
}