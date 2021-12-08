#ifndef HELENA_SYSTEMS_PLUGINMANAGER_HPP
#define HELENA_SYSTEMS_PLUGINMANAGER_HPP

#include <Helena/Engine/Engine.hpp>
#include <Helena/Types/FixedBuffer.hpp>

namespace Helena::Systems
{
    class PluginManager 
    {
    public:
        using PluginName   = Types::FixedBuffer<30>;

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
            Plugin(const PluginName& name, HELENA_MODULE_HANDLE handle, fnPluginInit fnInit, fnPluginEnd fnEnd, std::uint32_t hash, EState state)
                : m_Name{name}
                , m_Handle{handle}
                , m_fnInit{fnInit}
                , m_fnEnd{fnEnd}
                , m_Hash{hash}
                , m_State{state} {}
            ~Plugin() = default;
            Plugin(const Plugin&) = default;
            Plugin(Plugin&&) noexcept = default;
            Plugin& operator=(const Plugin&) = default;
            Plugin& operator=(Plugin&&) noexcept = default;

            PluginName m_Name;
            HELENA_MODULE_HANDLE m_Handle;
            fnPluginInit m_fnInit;
            fnPluginEnd m_fnEnd;
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

        [[nodiscard]] bool Load(const std::string_view path, const PluginName& name);
        [[nodiscard]] bool Load(const PluginName& name);
        [[nodiscard]] bool Init(const PluginName& name);
        [[nodiscard]] bool End(const PluginName& name);
        [[nodiscard]] bool Has(const PluginName& name) const noexcept;
        [[nodiscard]] bool IsInitialized(const PluginName& name) const noexcept;

        template <typename Func>
        void Each(Func callback) const;

    private:
        std::vector<Plugin> m_Plugins;
    };
}

namespace Helena::Events::PluginManager
{
    struct Load {
        const Systems::PluginManager::PluginName& name;
    };

    struct PluginInit {
        const Systems::PluginManager::PluginName& name;
    };

    struct PluginEnd {
        const Systems::PluginManager::PluginName& name;
    };
}

#include <Helena/Systems/PluginManager.ipp>

#endif // HELENA_SYSTEMS_PLUGINMANAGER_HPP
