#ifndef __COMMON_HFMODULE_HPP__
#define __COMMON_HFMODULE_HPP__

#include <iostream>
#include <unordered_map>
#include <string>

#include "HFPluginManager.hpp"

namespace Helena
{
    class HFApp;
    class HFPlugin;

    class HFModule
    {
        friend class HFApp;

    public:
        explicit HFModule() {
            HFPluginManager::m_pModule = this;
        }
        virtual ~HFModule() = default;

        HFModule(const HFModule&) = delete;
        HFModule(HFModule&&) = delete;
        HFModule& operator=(const HFModule&) = delete;
        HFModule& operator=(HFModule&&) = delete;
    
    protected:
        /*! @brief Called after success modules initialization */
        virtual bool AppInit()      { return true; }

        /** @brief Called after success AppInit, used for load configs before AppStart */
        virtual bool AppConfig()    { return true; }

        /*! @brief Called after success AppConfig, used for starting */
        virtual bool AppStart()     { return true; }

        /*! @brief Called after success AppStart, it's main HFApp loop */
        virtual bool AppUpdate()    { return true; }

        /*! @brief Called after success AppUpdate, used for free resources */
        virtual bool AppShut()      { return true; }

    protected:
        /*********************************
         * @brief Get pointer on HFApp
         * @return Pointer on HFAapp
         *********************************/ 
        HFApp* GetApp() const noexcept {
            return m_pApp;
        }

        /*! @brief copydoc AddPlugin */
        template <typename Plugin, typename... Args, typename = std::enable_if_t<std::is_base_of_v<HFPlugin, Plugin>>>
        Plugin* AddPlugin([[maybe_unused]] Args&&... args) {
            return HFPluginManager::AddPlugin<Plugin>(std::forward<Args>(args)...);
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

    private:
        HFApp* m_pApp;
    };
}

#endif // __COMMON_HFMODULE_HPP__