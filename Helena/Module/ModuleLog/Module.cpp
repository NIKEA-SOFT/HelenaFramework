#include <Common/Service.hpp>
#include <Include/PluginLog.hpp>

namespace Helena
{
    HF_API void HFMain(ModuleManager* pModuleManager, EModuleState state) 
    {
        switch(state)
        {
            case EModuleState::Init : {
                pModuleManager->CreatePlugin<IPluginLog, PluginLog>(EPluginPriority::HIGH);
            } break;

            case EModuleState::Free : {
                pModuleManager->RemovePlugin<IPluginLog>();
            } break;
        }
    }
}