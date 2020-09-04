#include <Common/ModuleManager.hpp>
#include <Include/PluginTestA.hpp>

namespace Helena
{
    HF_MAIN void HFMain(ModuleManager* pModuleManager, EModuleState state)
    {
        switch(state)
        {
            case EModuleState::Init:
            {
                pModuleManager->CreatePlugin<IPluginTestA, PluginTestA>(pModuleManager);

            } break;

            case EModuleState::Free:
            {
                pModuleManager->RemovePlugin<IPluginTestA>();
            } break;
        }
    }
}