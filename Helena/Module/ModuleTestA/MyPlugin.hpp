#ifndef __MODULETESTA_MYPLUGIN_H__
#define __MODULETESTA_MYPLUGIN_H__

#include <Helena/Common/HFPlugin.hpp>

namespace Helena
{
    class MyPlugin final : public HFPlugin 
    {
    public:
        void Foo();
    };
}

#endif // __MODULETESTA_MYPLUGIN_H__