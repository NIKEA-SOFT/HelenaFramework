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

        bool AppInit() override {
            std::cout << "TestModuleA AppInit call, my addr: " << this << std::endl;

            return true;
        }
    };

    class TestPluginA final : public HFPlugin
    {
    public:
        TestPluginA() {
            std::cout <<  "TestPluginA ctor" << std::endl;
        }
        ~TestPluginA() {
            std::cout <<  "TestPluginA dtor" << std::endl;
        }

        void Init() {
            std::cout << "TestPluginA init, my module: " << this->GetModule() << std::endl;
        }
    };

    class TestPluginB final : public HFPlugin
    {
    public:
        TestPluginB() {
            std::cout <<  "TestPluginB ctor" << std::endl;
        }
        ~TestPluginB() {
            std::cout <<  "TestPluginBdtor" << std::endl;
        }
    };
}

HF_API void HFMain(HFApp* pApp, HF_MODULE_STATE state) 
{
    switch(state)
    {
        case HF_MODULE_STATE::HF_MODULE_INIT : {   
            pApp->AddModule<TestModuleA>();
        } break;

        case HF_MODULE_STATE::HF_MODULE_FREE : {
            pApp->RemoveModule<TestModuleA>();
        } break;
    }
}