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
}

#endif // COMMON_META_HPP