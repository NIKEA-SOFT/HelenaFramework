#ifndef __MODULE_MODULETESTB_HPP__
#define __MODULE_MODULETESTB_HPP__

#include <Helena/Common/HFModule.hpp>

namespace Helena
{
    // Declaration Modules
    class ModuleTestA;

    // Declaration Plugins
    class PluginC;

    // Module class
    class HF_API ModuleTestB final : public HFModule
    {
    public:
        explicit ModuleTestB() : m_pModuleTestA(nullptr) {}
        ~ModuleTestB() = default;

    protected:
        bool AppInit() override;
        bool AppConfig() override;
        bool AppStart() override;
        bool AppUpdate() override;
        bool AppShut() override;

    public:
        void Hello();
        PluginC* GetPluginC() const;

    private:
        ModuleTestA* m_pModuleTestA;
    };
}
#endif // __MODULE_MODULETESTB_HPP__