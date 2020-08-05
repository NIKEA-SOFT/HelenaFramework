#ifndef COMMON_HFMODULE_HPP
#define COMMON_HFMODULE_HPP

#include <iostream>
#include <vector>
#include <string>
#include "HFPlugin.hpp"

namespace Helena
{
    class HFApp;
    class HFModule
    {

    public:
        HFModule() = default;
        virtual ~HFModule() = default;

        virtual bool AppInit()      { return true; }
        virtual bool AppConfig()    { return true; }
        virtual bool AppStart()     { return true; }
        virtual bool AppUpdate()    { return true; }
        virtual bool AppShut()      { return true; }
        
    private:
        
    };
}

#endif