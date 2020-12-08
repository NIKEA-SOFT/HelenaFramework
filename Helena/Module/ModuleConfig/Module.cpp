#include <Common/Service.hpp>
#include <Include/PluginConfig.hpp>

namespace Helena
{
    HF_API void HFMain(ModuleManager* pModuleManager, EModuleState state) 
    {
        switch(state)
        {
            case EModuleState::Init : {
                pModuleManager->CreatePlugin<IPluginConfig, PluginConfig>();
            } break;

            case EModuleState::Free : {
                pModuleManager->RemovePlugin<IPluginConfig>();
            } break;
        }
    }
}