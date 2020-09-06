#ifndef COMMON_META_HPP
#define COMMON_META_HPP

#include <string_view>

namespace Helena::Meta
{
    #define META_TYPE   static constexpr auto
    #define REG_META(arg) {                         \
        constexpr const char* x{arg};               \
        return x;                                   \
    }                                               \

    struct ConfigService
    {
        // Applications
        META_TYPE Service()         REG_META("Service")
        META_TYPE PathConfigs()     REG_META("PathConfigs")
        META_TYPE PathModules()     REG_META("PathModules")
        META_TYPE PathResources()   REG_META("PathResources")
        META_TYPE Modules()         REG_META("Modules")
    };

    struct ConfigLogger
    {
        META_TYPE ConfigFile()      REG_META("Logger.xml")
        META_TYPE Service()         REG_META("Service")
        META_TYPE Name()            REG_META("Name")
        META_TYPE Level()           REG_META("Level")
        META_TYPE Format()          REG_META("Format")
        META_TYPE Size()            REG_META("Size")
        META_TYPE Thread()          REG_META("Thread")
        META_TYPE Path()            REG_META("Path")
    };
}

#endif // COMMON_META_HPP