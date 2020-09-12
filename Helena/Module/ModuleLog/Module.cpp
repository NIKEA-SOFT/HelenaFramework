#include <Common/ModuleManager.hpp>

#include <Include/PluginLog.hpp>

namespace Helena
{
    HF_API void HFMain(ModuleManager* pModuleManager, EModuleState state) 
    {
        switch(state)
        {
            case EModuleState::Init : {
                pModuleManager->CreatePlugin<IPluginLog, PluginLog>();
            } break;

            case EModuleState::Free : {
                pModuleManager->RemovePlugin<IPluginLog>();
            } break;
        }
    }
}