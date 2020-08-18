#include "MyPlugin.hpp"

#include "Module.hpp"

namespace Helena
{
    void MyPlugin::Foo() 
    {
        auto myModule = this->GetModule();

        if(myModule) {
            std::cout << "Get ptr on Module success!" << std::endl;
            if(auto myPlugin = this->GetPlugin<MyPlugin>(); myPlugin) {
                std::cout << "Get ptr on MyPlugin from Plugin class success" << std::endl;
            }
        }
        
    }
}