#ifndef __COMMON_HFPLUGIN_HPP__
#define __COMMON_HFPLUGIN_HPP__

#include "HFModuleManager.hpp"
#include "HFPluginManager.hpp"

namespace Helena
{
    class HFPlugin
    {
    public:
        virtual ~HFPlugin() = default;

    protected:
        /*! @brief copydoc AddPlugin */
        template <typename Plugin, typename... Args, typename = std::enable_if_t<std::is_base_of_v<HFPlugin, Plugin>>>
        void AddPlugin([[maybe_unused]] Args&&... args) {
            HFPluginManager::AddPlugin<Plugin>(std::forward<Args>(args)...);
        }

        /*! @brief copydoc GetPlugin */
        template <typename Plugin, typename = std::enable_if_t<std::is_base_of_v<HFPlugin, Plugin>>>
        Plugin* GetPlugin() const noexcept {
            return HFPluginManager::GetPlugin<Plugin>();
        }

        /*! @brief copydoc RemovePlugin */
        template <typename Plugin, typename = std::enable_if_t<std::is_base_of_v<HFPlugin, Plugin>>>
        void RemovePlugin() noexcept {
            return HFPluginManager::RemovePlugin<Plugin>();
        }

        template <typename Base, typename Module, typename = std::enable_if_t<std::is_base_of_v<Base, Module>>>
        static Base* GetModule() noexcept {
            return HFModuleManager::GetModule<Base, Module>();
        }
    };
}

#endif // __COMMON_HFPLUGIN_HPP__