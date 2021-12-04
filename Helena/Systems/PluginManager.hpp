#ifndef HELENA_SYSTEMS_PLUGINMANAGER_HPP
#define HELENA_SYSTEMS_PLUGINMANAGER_HPP

#include <Helena/Engine/Engine.hpp>
#include <Helena/Types/FixedBuffer.hpp>

namespace Helena::Systems
{
    class PluginManager 
    {
    public:
        using FixedBuffer   = Types::FixedBuffer<30>;

    private:
        static constexpr const char* KPluginInit    = "PluginInit";
        static constexpr const char* KPluginEnd     = "PluginEnd";

        using fnPluginInit  = void (*)(std::shared_ptr<Engine::Context>);
        using fnPluginEnd   = void (*)();

        enum class EState : std::uint8_t {
            Loaded,
            Init
        };

        struct Plugin {
            FixedBuffer m_Name;
            HELENA_MODULE_HANDLE m_Handle;
            fnPluginInit m_fnPluginInit;
            fnPluginEnd m_fnPluginEnd;
            std::uint32_t m_Hash;
            EState m_State;
        };

        [[nodiscard]] auto Find(std::uint32_t hash) const noexcept;
        [[nodiscard]] auto Find(std::uint32_t hash) noexcept;
        
    public:
        PluginManager() = default;
        ~PluginManager();
        PluginManager(const PluginManager&) = delete;
        PluginManager(PluginManager&&) noexcept = delete;
        PluginManager& operator=(const PluginManager&) = delete;
        PluginManager& operator=(PluginManager&&) noexcept = delete;

        [[nodiscard]] bool Load(const std::string_view path, const FixedBuffer& name);
        [[nodiscard]] bool PluginInit(const FixedBuffer& name);
        [[nodiscard]] bool PluginEnd(const FixedBuffer& name);
        [[nodiscard]] bool Has(const FixedBuffer& name) const noexcept;
        [[nodiscard]] bool IsInitialized(const FixedBuffer& name) const noexcept;

        template <typename Func>
        void Each(Func callback) const;

    private:
        std::vector<Plugin> m_Plugins;
    };
}

namespace Helena::Events::PluginManager
{
    struct Load {
        const Systems::PluginManager::FixedBuffer& name;
    };

    struct PluginInit {
        const Systems::PluginManager::FixedBuffer& name;
    };

    struct PluginEnd {
        const Systems::PluginManager::FixedBuffer& name;
    };
}

#include <Helena/Systems/PluginManager.ipp>

#endif // HELENA_SYSTEMS_PLUGINMANAGER_HPP
