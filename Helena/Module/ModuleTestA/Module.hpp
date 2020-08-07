#include <Helena/Common/HFApp.hpp>

namespace Helena
{
    class ModuleTestA : public HFModule
    {
    public:
        bool AppInit() override;
        bool AppConfig() override;
        bool AppStart() override;
        bool AppUpdate() override;
        bool AppShut() override;
    };
}