#ifndef COMMON_IPLUGIN_HPP
#define COMMON_IPLUGIN_HPP

#include "Platform.hpp"

namespace Helena
{
    class IPlugin
    {
    public:
        virtual ~IPlugin() = default;

        virtual bool Initialize() {
            return true;
        }

        virtual bool Config() {
            return true;
        }

        virtual bool Execute() {
            return true;
        }

        virtual bool Update() {
            return true;
        }

        virtual bool Finalize() {
            return true;
        }
    };
}

#endif // COMMON_IPLUGIN_HPP