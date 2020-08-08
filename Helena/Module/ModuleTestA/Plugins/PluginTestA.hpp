#ifndef __MODULETESTA_PLUGINTESTA_H__
#define __MODULETESTA_PLUGINTESTA_H__

#include <Helena/Common/HFPlugin.hpp>

namespace Helena
{
    class PluginTestA : public HFPlugin
    {
    public:
        uint32_t m_Value {99};
    };
}

#endif // __MODULETESTA_PLUGINTESTA_H__