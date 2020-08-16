#ifndef __MODULE_MODULETESTB_HPP__
#define __MODULE_MODULETESTB_HPP__

#include <Helena/Common/HFModule.hpp>

namespace Helena
{
    // Declaration
    class ModuleTestA;

    // Module class
    class ModuleTestB final : public HFModule
    {
    public:
        explicit ModuleTestB() : m_pModuleTestA(nullptr) {}
        ~ModuleTestB() = default;

        bool AppInit() override;
        bool AppConfig() override;
        bool AppStart() override;
        bool AppUpdate() override;
        bool AppShut() override;

    private:
        ModuleTestA* m_pModuleTestA;
    };
}
#endif // __MODULE_MODULETESTB_HPP__