#include <Common/ModuleManager.hpp>
#include <Include/PluginTestA.hpp>

namespace Helena
{

    // EntryPoint in module (dll/so)
    // ModuleManager is used to manage plugin instances
    // and also to get paths or the name of the current service
    HF_API void HFMain(ModuleManager* pModuleManager, EModuleState state)
    {
        // Order of processing module logic:
        // 1) Load modules (dll/so) to ModuleManager
        // 2) Calling HFMain with Init state
        // 3) Virtual calls in modules: Initialize, Config, Execute, Update, Finalize
        // 4) Calling HFMain with Free state 
        // 5) Unload modules (dll/so) from ModuleManager

        switch(state)
        {
            case EModuleState::Init:
            {
                // Create an object (allocate memory) of class PluginTestA with abstract class IPluginTestA
                // An abstract class is used to expose methods of the PluginTestA class to third-party modules.
                // It may sound confusing, but it's actually not difficult.
                // The CreatePlugin method accepts arguments that can be passed to 
                // the constructor of the PluginTestA class.
                pModuleManager->CreatePlugin<IPluginTestA, PluginTestA>();
            } break;

            case EModuleState::Free:
            {
                // Remove and free the previously created PluginTestA class object
                // If your destructor uses third-party plugins, 
                // then you must follow the order of calls to RemovePlugin.
                pModuleManager->RemovePlugin<IPluginTestA>();
            } break;
        }
    }
}