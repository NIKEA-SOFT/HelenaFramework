#include "Helena/Common/HFApp.hpp"

#ifdef HF_PLATFORM_WIN
    #pragma comment(lib, "DbgHelp.lib")
#endif

using namespace Helena;

namespace Helena
{
    class TestModuleB final : public HFModule
    {
    public:
        
    };
}

HF_API void HFMain(HFApp* pApp, HF_MODULE_STATE state) 
{
    switch(state)
    {
        case HF_MODULE_STATE::HF_MODULE_INIT : {   
            pApp->AddModule<TestModuleB>();
        } break;

        case HF_MODULE_STATE::HF_MODULE_FREE : {
            pApp->RemoveModule<TestModuleB>();
        } break;
    }
}