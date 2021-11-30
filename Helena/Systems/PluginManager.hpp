#ifndef HELENA_SYSTEMS_PLUGINMANAGER_HPP
#define HELENA_SYSTEMS_PLUGINMANAGER_HPP

#include <Helena/Types/FixedBuffer.hpp>

#include <vector>

namespace Helena::Systems
{
    class PluginManager 
    {
        using FixedBuffer = Types::FixedBuffer<32>;

        struct Plugin {
            FixedBuffer m_Name;
            std::uint32_t m_Hash;
        };

        [[nodiscard]] auto Find(const FixedBuffer& name) const noexcept;

    public:
        PluginManager() = default;
        ~PluginManager() = default;
        PluginManager(const PluginManager&) = delete;
        PluginManager(PluginManager&&) noexcept = delete;
        PluginManager& operator=(const PluginManager&) = delete;
        PluginManager& operator=(PluginManager&&) noexcept = delete;

        void Create(const FixedBuffer& name);
        //void Remove(const FixedBuffer& name);

    private:
        std::vector<Plugin> m_Plugins;
    };
}

#include <Helena/Systems/PluginManager.ipp>

#endif // HELENA_SYSTEMS_PLUGINMANAGER_HPP
