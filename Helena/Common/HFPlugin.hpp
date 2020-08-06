#ifndef COMMON_HFPLUGININFO_HPP
#define COMMON_HFPLUGININFO_HPP

#include "HFPlatform.hpp"

namespace Helena
{
    class HFPlugin
    {
        friend class HFModule;
    public:
        virtual ~HFPlugin() = default;

        HFModule* GetModule() {
            return this->m_pModule;
        }

    private:
        HFModule* m_pModule;
    };
}

#endif