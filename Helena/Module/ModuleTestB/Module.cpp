#include <Common/Service.hpp>
#include <Include/PluginTestB.hpp>

namespace Helena
{
    HF_API void HFMain(ModuleManager* pModuleManager, EModuleState state)
    {
        switch(state)
        {
            case EModuleState::Init: {
                pModuleManager->CreatePlugin<IPluginTestB, PluginTestB>(EPluginPriority::LOW);
            } break;

            case EModuleState::Free: {
                pModuleManager->RemovePlugin<IPluginTestB>();
            } break;
        }
    }
}