#ifndef COMMON_HFPLUGININFO_HPP
#define COMMON_HFPLUGININFO_HPP

#include "HFPlatform.hpp"

namespace Helena
{
    class HFPlugin
    {
        friend class HFModule;
    public:
        /*! @brief Virtual dtor for correctly free allocated memory */
        virtual ~HFPlugin() = default;

        /**
         * @brief Return pointer on module, where registered plugin
         * @return Stable pointer on HFModule or nullptr 
         * if trying use it from plugin ctor
         */
        HFModule* GetModule() {
            return this->m_pModule;
        }

    private:
        HFModule* m_pModule;
    };
}

#endif