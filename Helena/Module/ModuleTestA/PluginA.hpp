#ifndef __MODULETESTA_PLUGINA_HPP__
#define __MODULETESTA_PLUGINA_HPP__

#include <Helena/Common/HFPlugin.hpp>

namespace Helena
{
    class HF_API PluginA final : public HFPlugin 
    {
    public:
        void Foo();
    };
}

#endif // __MODULETESTA_PLUGINA_HPP__