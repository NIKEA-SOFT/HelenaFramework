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
        TestModuleA() {
            std::cout <<  "TestModuleA ctor" << std::endl;
        }
        ~TestModuleA() {
            std::cout <<  "TestModuleA dtor" << std::endl;
        }
    };

    class TestModuleB final : public HFModule
    {
    public:
        TestModuleB() {
            std::cout <<  "TestModuleA ctor" << std::endl;
        }
        ~TestModuleB() {
            std::cout <<  "TestModuleA dtor" << std::endl;
        }
    };
}

HF_API void HFMain(HFApp* pApp, HF_MODULE_STATE state) 
{
    switch(state)
    {
        case HF_MODULE_STATE::HF_MODULE_INIT : 
        {   
            pApp->AddModule<TestModuleA>();
        } break;

        case HF_MODULE_STATE::HF_MODULE_FREE : 
        {
            pApp->RemoveModule<TestModuleA>();
        }
    }
}