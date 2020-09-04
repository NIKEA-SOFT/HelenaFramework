#include <Common/ModuleManager.hpp>

#include <Include/PluginLog.hpp>

namespace Helena
{
    HF_MAIN void HFMain(ModuleManager* pModuleManager, EModuleState state) 
    {
        switch(state)
        {
            case EModuleState::Init : 
            {
                pModuleManager->CreatePlugin<IPluginLog, PluginLog>(pModuleManager);
                
            } break;

            case EModuleState::Free : 
            {
                pModuleManager->RemovePlugin<IPluginLog>();
            } break;
        }
    }
}