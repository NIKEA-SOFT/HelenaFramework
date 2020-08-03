#include "Helena/Common/HFApp.hpp"

#ifdef HF_PLATFORM_WIN
    #pragma comment(lib, "DbgHelp.lib")
#endif

using namespace Helena;

namespace Helena
{
    class TestModuleA final : public HFModule
    {
    public:
    };

    class TestModuleB final : public HFModule
    {
    public:
        
    };
}

HF_API bool HFMain(HFApp* pApp, HF_MODULE_STATE state) 
{
    switch(state)
    {
        case HF_MODULE_STATE::HF_MODULE_INIT : 
        {   
            pApp->SetObject();
            if(!pApp->AddModule<TestModuleA>()) {
                return false;
            }

            if(!pApp->AddModule<TestModuleA>()) {
                return false;
            }
        } break;

        case HF_MODULE_STATE::HF_MODULE_FREE : 
        {

        }
    }
    return true;
}