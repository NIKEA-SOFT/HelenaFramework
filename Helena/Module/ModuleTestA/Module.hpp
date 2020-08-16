#ifndef __MODULE_MODULETESTA_HPP__
#define __MODULE_MODULETESTA_HPP__

#include <Helena/Common/HFModule.hpp>
#include <Helena/Module/ModuleTestA/Plugins/PluginTestA.hpp>

namespace Helena
{
    // Declaration
    class ModuleTestB;
    class ModuleLog;

    // Module class
    class ModuleTestA final : public HFModule
    {
    public:
        explicit ModuleTestA()
    	: m_pModuleTestB(nullptr)
    	, m_pModuleLog(nullptr) {}
        ~ModuleTestA()      override = default;
    	
        bool AppInit()      override;
        bool AppConfig()    override;
        bool AppStart()     override;
        bool AppUpdate()    override;
        bool AppShut()      override;

    private:
        ModuleTestB*    m_pModuleTestB;
        ModuleLog*      m_pModuleLog;

    	
    private:
        friend class PluginTestA;
    	
    	ModuleTestB* GetModuleTestB() const;
    };
}
#endif // __MODULE_MODULETESTA_HPP__