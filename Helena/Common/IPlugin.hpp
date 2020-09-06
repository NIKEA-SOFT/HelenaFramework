#ifndef COMMON_IPLUGIN_HPP
#define COMMON_IPLUGIN_HPP

namespace Helena
{
    class IPlugin
    {
    public:
        IPlugin() = default;
        virtual ~IPlugin() = default;
        IPlugin(const IPlugin&) = delete;
        IPlugin(IPlugin&&) = delete;
        IPlugin& operator=(const IPlugin&) = delete;
        IPlugin& operator=(IPlugin&&) = delete;

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