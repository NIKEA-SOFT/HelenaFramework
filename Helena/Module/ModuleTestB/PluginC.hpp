#ifndef __MODULETESTB_PLUGINC_HPP__
#define __MODULETESTB_PLUGINC_HPP__

#include <Helena/Common/HFPlugin.hpp>

namespace Helena
{
    class HF_API PluginC final : public HFPlugin 
    {
    public:
        void Zoo() const;
    };
}

#endif // __MODULETESTB_PLUGINC_HPP__