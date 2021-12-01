#ifndef HELENA_SYSTEMS_PLUGINMANAGER_HPP
#define HELENA_SYSTEMS_PLUGINMANAGER_HPP

#include <Helena/Engine/Engine.hpp>
#include <Helena/Types/FixedBuffer.hpp>

namespace Helena::Systems
{
    class PluginManager 
    {
    public:
        using FixedBuffer   = Types::FixedBuffer<32>;

    private:
        static constexpr const char* fnPluginInit   = "PluginInit";
        static constexpr const char* fnPluginEnd    = "PluginEnd";

        struct Plugin {
            FixedBuffer m_Name;
            HELENA_MODULE_HANDLE m_Handle;
            std::uint32_t m_Hash;
        };

        [[nodiscard]] auto Find(std::uint32_t hash) const noexcept;
        
    public:
        PluginManager() = default;
        ~PluginManager();
        PluginManager(const PluginManager&) = delete;
        PluginManager(PluginManager&&) noexcept = delete;
        PluginManager& operator=(const PluginManager&) = delete;
        PluginManager& operator=(PluginManager&&) noexcept = delete;

        [[nodiscard]] bool Create(const FixedBuffer& name);
        void Remove(const FixedBuffer& name);

    private:
        std::vector<Plugin> m_Plugins;
    };
}

namespace Helena::Events::PluginManager
{
    struct Create {
        const Systems::PluginManager::FixedBuffer& name;
    };

    struct Remove {
        const Systems::PluginManager::FixedBuffer& name;
    };
}

#include <Helena/Systems/PluginManager.ipp>

#endif // HELENA_SYSTEMS_PLUGINMANAGER_HPP
