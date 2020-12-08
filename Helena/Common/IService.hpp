#ifndef COMMON_IPLUGIN_HPP
#define COMMON_IPLUGIN_HPP

namespace Helena
{
    class IPlugin
    {
        // Friend is used to get a clean interface without Set methods
        friend class ModuleManager;
        
    public:
        IPlugin() : m_pModuleManager(nullptr) {}
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

    public:
        [[nodiscard]] ModuleManager* GetModuleManager() const noexcept {
            return m_pModuleManager;
        }

    private:
        ModuleManager* m_pModuleManager;
    };
}

#endif // COMMON_IPLUGIN_HPP