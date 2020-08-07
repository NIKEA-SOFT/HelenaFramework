#include "Module.hpp"

using Helena::HFApp;
using Helena::HF_MODULE_STATE;
using Helena::ModuleTestB;

HF_API void HFMain(HFApp* pApp, HF_MODULE_STATE state) 
{
    switch(state)
    {
        case HF_MODULE_STATE::HF_MODULE_INIT : {   
            pApp->AddModule<ModuleTestB>();
        } break;

        case HF_MODULE_STATE::HF_MODULE_FREE : {
            pApp->RemoveModule<ModuleTestB>();
        } break;
    }
}