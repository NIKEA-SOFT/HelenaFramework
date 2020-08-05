#ifndef COMMON_HFMETA_HPP
#define COMMON_HFMETA_HPP

#include <string_view>

namespace Helena::Meta
{
    #define META_TYPE   static constexpr auto
    #define REG_META(arg) {                         \
        constexpr const std::string_view x(arg);    \
        return x;                                   \
    }                                               \

    struct Helena
    {
        // Config file
        META_TYPE GetConfig()               REG_META("Helena.xml")

        // Applications
        META_TYPE GetApplications()         REG_META("Applications")        // Node
        META_TYPE GetApp()                  REG_META("App")                 // Node
        META_TYPE GetModule()               REG_META("Module")              // Node
        META_TYPE GetLog()                  REG_META("Log")                 // Node

        META_TYPE GetName()                 REG_META("name")                // Attribute
        META_TYPE GetPath()                 REG_META("path")                // Attribute
        META_TYPE GetAuthor()               REG_META("author")              // Attribute
        META_TYPE GetVersion()              REG_META("version")             // Attribute
        META_TYPE GetLevel()                REG_META("level")               // Attribute
        META_TYPE GetFormat()               REG_META("format")              // Attribute
        META_TYPE GetSize()                 REG_META("size")                // Attribute
        META_TYPE GetThread()               REG_META("thread")              // Attribute
    };
}

#endif