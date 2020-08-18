#ifndef __COMMON_HFPLUGINMANAGER_H__
#define __COMMON_HFPLUGINMANAGER_H__

#include <vector>
#include <type_traits>

#include "HFPlatform.hpp"

namespace Helena
{
    // forward definition
    class HFModule;
    class HFPlugin;

    /*! @brief Manager plugins in dll memory space (friendly interface of class) */
    class HFPluginManager 
    {
        using PluginID = uint32_t;

        /*! @brief Type index sequencer */
        struct type_index_seq { 
            [[nodiscard]] static PluginID next() noexcept {
                static PluginID id {};
                return id++;
            }
        };

        /*! @brief Type index */
        template <typename Type, typename = void>
        struct type_index {
            [[nodiscard]] static PluginID id() noexcept {
                static const PluginID id = type_index_seq::next();
                return id;
            }
        };

        template <typename, typename = void>
        struct is_indexable : std::false_type {};

        template <typename Type>
        struct is_indexable<Type, std::void_t<decltype(type_index<Type>::id())>> : std::true_type {};
    
        template <typename Type>
        static constexpr bool is_indexable_v = is_indexable<Type>::value;

    public:
        /****************************************************************
         * @brief Initialize a plugin instance of the type
         * @tparam Plugin Type of plugin
         * @tparam Args Type of arguments to forward to the plugin ctor
         * @param args Arguments to forward to the plugin ctor
         * @return Pointer on created plugin or nullptr 
         * if plugin already has or memory not allocated
         ****************************************************************/
        template <typename Plugin, typename... Args, typename = std::enable_if_t<std::is_base_of_v<HFPlugin, Plugin>>>
        static Plugin* AddPlugin([[maybe_unused]] Args&&... args) {
            static_assert(is_indexable_v<Plugin>);
            const auto index = type_index<Plugin>::id();

            if(!(index < m_Plugins.size())) {
                m_Plugins.resize(index + 1);
            }

            if(auto& pPlugin = m_Plugins[index]; !pPlugin) {
                pPlugin = HF_NEW Plugin(std::forward<Args>(args)...);
                return static_cast<Plugin*>(pPlugin);
            }
            return nullptr;
        }

        /****************************************************************
         * @brief Get a pointer to a plugin instance of the type
         * @tparam Plugin Type of plugin
         * @return Pointer on created plugin or nullptr 
         * if plugin already has or memory not allocated
         ****************************************************************/      
        template <typename Plugin, typename = std::enable_if_t<std::is_base_of_v<HFPlugin, Plugin>>>
        static Plugin* GetPlugin() noexcept {
            static_assert(is_indexable_v<Plugin>);
            const auto index = type_index<Plugin>::id();
            if(index < m_Plugins.size()) {
                return static_cast<Plugin*>(m_Plugins[index]);
            }
            return nullptr;
        }

        /****************************************************************
         * @brief Remove a plugin instance of the type
         * @tparam Plugin Type of plugin
         ****************************************************************/  
        template <typename Plugin, typename = std::enable_if_t<std::is_base_of_v<HFPlugin, Plugin>>>
        static void RemovePlugin() noexcept {
            static_assert(is_indexable_v<Plugin>);
            const auto index = type_index<Plugin>::id();
            if(index < m_Plugins.size()) {
                HF_FREE(m_Plugins[index])
            }
        }

        inline static std::vector<HFPlugin*> m_Plugins;
        inline static HFModule* m_pModule = nullptr;
    };
}

#endif // __COMMON_HFPLUGINMANAGER_H__