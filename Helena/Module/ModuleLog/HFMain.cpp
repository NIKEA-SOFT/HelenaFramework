#include <Helena/Common/HFApp.hpp>
#include "Module.hpp"

namespace Helena
{
    HF_API void HFMain(HFApp* pApp, HF_MODULE_STATE state) 
    {
        switch(state)
        {
            case HF_MODULE_STATE::HF_MODULE_INIT : {   
                pApp->AddModule<ModuleLog>();
            } break;

            case HF_MODULE_STATE::HF_MODULE_FREE : {
                pApp->RemoveModule<ModuleLog>();
            } break;
        }
    }
}