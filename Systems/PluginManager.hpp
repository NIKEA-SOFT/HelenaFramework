#ifndef HELENA_SYSTEMS_PLUGINMANAGER_HPP
#define HELENA_SYSTEMS_PLUGINMANAGER_HPP

#include <Helena/Types/Hash.hpp>
#include <Helena/Types/ModernDesign.hpp>

#include <filesystem>

namespace Helena::Events::PluginManager
{
    struct PreLoad {
        std::string_view name;
    };

    struct PostLoad {
        std::string_view name;
        bool error;
    };

    struct PreReload {
        std::string_view name;
    };

    struct PostReload {
        std::string_view name;
        bool error;
    };

    struct PreUnload {
        std::string_view name;
    };

    struct PostUnload {
        std::string_view name;
    };
}

namespace Helena::Systems
{
    class PluginManager final : public Types::ModernDesign<PluginManager>
    {
    public:
        enum class EState : std::uint8_t {
            Load,
            Reload,
            Unload
        };

    private:
        static constexpr const auto m_EntryPoint = "PluginMain";
        static constexpr const auto m_Extension = ".plugin";

        using EntryPoint = void (EState, Engine::Context&);

    public:
        PluginManager(const std::filesystem::path& directory) : m_Directory{directory}
        {
            [[maybe_unused]] std::error_code err;
            m_Directory = std::filesystem::absolute(std::filesystem::canonical(m_Directory, err));
            HELENA_ASSERT(std::filesystem::is_directory(directory, err),
                "Path: \"{}\" is not directory, error: {}, message: {}", m_Directory.string(), err.value(), err.message());

            Helena::Engine::SubscribeEvent<Events::Engine::PostShutdown>(&PluginManager::OnPostShutdown);
        }
        ~PluginManager() {
            Helena::Engine::UnsubscribeEvent<Events::Engine::PostShutdown>(&PluginManager::OnPostShutdown);
        }

        PluginManager(const PluginManager&) = delete;
        PluginManager(PluginManager&&) noexcept = delete;
        PluginManager& operator=(const PluginManager&) = delete;
        PluginManager& operator=(PluginManager&&) noexcept = delete;

        [[nodiscard]] static bool Load(std::string_view pluginName)
        {
            auto& self = CurrentSystem();
            if(pluginName.ends_with(HELENA_MODULE_EXTENSION)) {
                HELENA_MSG_ERROR("Plugin: \"{}\" contain extension, extension is added automatically", pluginName);
                return false;
            }

            if(self.m_Plugins.contains(pluginName)) {
                HELENA_MSG_ERROR("Plugin: \"{}\" already loaded", pluginName);
                return false;
            }

            std::error_code err;
            if(!std::filesystem::is_directory(self.m_Directory, err)) {
                HELENA_MSG_ERROR("Path: \"{}\" is not directory, error: {}, message: {}",
                    self.m_Directory.string(), err.value(), err.message());
                return false;
            }

            auto plugin = std::string{pluginName} + HELENA_MODULE_EXTENSION;
            auto pluginPath = self.m_Directory / plugin; plugin.resize(plugin.size() - (sizeof(HELENA_MODULE_EXTENSION) - 1 /*null*/));
            if(!std::filesystem::is_regular_file(pluginPath, err)) {
                HELENA_MSG_ERROR("Plugin: \"{}\" plugin not found, error: {}, message: {}",
                    pluginPath.string(), err.value(), err.message());
                return false;
            }

            {
                const auto oldPath = pluginPath; pluginPath.replace_extension(m_Extension);
                if(!std::filesystem::copy_file(oldPath, pluginPath, std::filesystem::copy_options::overwrite_existing, err)) {
                    HELENA_MSG_ERROR("Copy plugin: \"{}\" to \"{}\" failed, error: {}, message: {}",
                        oldPath.string(), pluginPath.string(), err.value(), err.message());
                    return false;
                }
            }

            const auto handle = LoadPlugin(pluginPath.string());
            HELENA_ASSERT(handle != nullptr, "Load plugin: \"{}\" failed", pluginPath.string());
            if(!handle) {
                std::filesystem::remove(pluginPath, err);
                HELENA_MSG_ERROR("Load plugin: \"{}\" failed", pluginPath.string());
                return false;
            }

            Engine::SignalEvent<Events::PluginManager::PreLoad>(pluginName);
            auto [it, _] = self.m_Plugins.emplace(std::piecewise_construct, std::forward_as_tuple(std::move(plugin)), std::forward_as_tuple(handle));
            if(!PluginMain(handle, EState::Load)) {
                Engine::SignalEvent<Events::PluginManager::PostLoad>(pluginName, /*error*/ true);
                UnloadPlugin(handle);
                RemovePlugin(pluginPath);
                self.m_Plugins.erase(it);
                HELENA_MSG_ERROR("Plugin: \"{}\", EP: \"{}\" symbol not exist", pluginName, m_EntryPoint);
                return false;
            }
            Engine::SignalEvent<Events::PluginManager::PostLoad>(pluginName, /*error*/ false);
            return true;
        }

        static void Unload(std::string_view pluginName)
        {
            auto& self = CurrentSystem();
            if(const auto it = self.m_Plugins.find(pluginName); it != self.m_Plugins.cend()) {
                Engine::SignalEvent<Events::PluginManager::PreUnload>(pluginName);
                PluginMain(it->second, EState::Unload);
                UnloadPlugin(it->second);
                RemovePlugin((self.m_Directory / pluginName) += m_Extension);
                self.m_Plugins.erase(it);
                Engine::SignalEvent<Events::PluginManager::PostUnload>(pluginName);
                return;
            }

            HELENA_MSG_WARNING("Plugin: \"{}\" not loaded, unload failed", pluginName);
        }

        [[nodiscard]] static bool Reload(std::string_view pluginName)
        {
            const auto& self = CurrentSystem();
            if(const auto it = self.m_Plugins.find(pluginName); it != self.m_Plugins.cend()) {
                Engine::SignalEvent<Events::PluginManager::PreReload>(pluginName);
                PluginMain(it->second, EState::Reload);
                Unload(pluginName);
                const auto result = Load(pluginName);
                Engine::SignalEvent<Events::PluginManager::PostReload>(pluginName, !result);
                return result;
            }

            HELENA_MSG_ERROR("Plugin: \"{}\" not loaded, reload failed", pluginName);
            return false;
        }

        [[nodiscard]] static bool Has(std::string_view pluginName) {
            return CurrentSystem().m_Plugins.contains(pluginName);
        }

    private:
        void OnPostShutdown()
        {
            auto begin = m_Plugins.begin();
            while(begin != m_Plugins.end()) {
                /*copy, because it is invalid after erasing*/
                auto [name, handle] = *begin;
                Engine::SignalEvent<Events::PluginManager::PreUnload>(name);
                PluginMain(handle, EState::Unload);
                UnloadPlugin(handle);
                RemovePlugin((m_Directory / name) += m_Extension);
                begin = m_Plugins.erase(begin);
                Engine::SignalEvent<Events::PluginManager::PostUnload>(name);
            }
        }

        static auto PluginMain(HELENA_MODULE_HANDLE handle, EState state) -> EntryPoint*
        {
            if(const auto ep = reinterpret_cast<EntryPoint*>(HELENA_MODULE_GETSYM(handle, m_EntryPoint))) {
                ep(state, Engine::GetContext<Engine::Context>());
                return ep;
            }

            return nullptr;
        }

        static auto LoadPlugin(std::string_view plugin) -> HELENA_MODULE_HANDLE {
            return static_cast<HELENA_MODULE_HANDLE>(HELENA_MODULE_LOAD(plugin.data()));
        }

        static void UnloadPlugin(HELENA_MODULE_HANDLE handle) {
            HELENA_MODULE_UNLOAD(handle);
        }

        static void RemovePlugin(const std::filesystem::path& path) {
            std::error_code err{};
            std::filesystem::remove(path, err);
        }

    private:
        std::unordered_map<std::string, HELENA_MODULE_HANDLE, Types::Hasher<std::string>, std::equal_to<>> m_Plugins;
        std::filesystem::path m_Directory;
    };
}

#endif // HELENA_SYSTEMS_PLUGINMANAGER_HPP
