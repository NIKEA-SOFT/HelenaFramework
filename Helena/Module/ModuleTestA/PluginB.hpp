#ifndef __MODULETESTA_PLUGINB_HPP__
#define __MODULETESTA_PLUGINB_HPP__

#include <Helena/Common/HFPlugin.hpp>

namespace Helena
{
    class HF_API PluginB final : public HFPlugin 
    {
    public:
        void Boo();
    };
}

#endif // __MODULETESTA_PLUGINB_HPP__