#ifndef __COMMON_HFMODULE_HPP__
#define __COMMON_HFMODULE_HPP__

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include "HFPlugin.hpp"

#if HF_STANDARD_VER > HF_STANDARD_CPP17
#include "HFHash.hpp"
#endif

namespace Helena
{
    class HFModule
    {
        friend class HFApp;
    
        using PluginId = uint32_t;
        /*! @breif Sequenced indexer */
        struct type_index_seq {
            /**
             * @brief Return next sequenced index
             * @return Sequenced index
             */
            [[nodiscard]] static PluginId next() noexcept {
                static PluginId id {};
                return id++;
            }
        };
        
        /**
         * @brief Type Index
         * @tparam Type Type for which to generate a sequential identifier.
         */
        template <typename Type, typename = void>
        struct type_index {
            /**
             * @brief Returns the sequential identifier of a given type.
             * @return The sequential identifier of a given type.
             */
            [[nodiscard]] static PluginId id() noexcept {
                static const PluginId id = type_index_seq::next();
                return id;
            }
        };
        
        /**
         * @brief Provides the member constant `value` to true if a given type is
         * indexable, false otherwise.
         * @tparam Type Potentially indexable type.
         */
        template <typename, typename = void>
        struct is_indexable : std::false_type {};

        /*! @brief has_type_index */
        template <typename Type>
        struct is_indexable<Type, std::void_t<decltype(type_index<Type>::id())>> : std::true_type {};

        /**
         * @brief Helper variable template.
         * @tparam Type Potentially indexable type.
         */
        template <typename Type>
        static constexpr bool is_indexable_v = is_indexable<Type>::value;

    public:
        HFModule() : m_pApp(nullptr) {};

        /*! @brief Virtual dtor for correctly free allocated memory */
        virtual ~HFModule() {
            for(auto& pPlugin : this->m_Plugins) {
                HF_FREE(pPlugin);
            }
        }
        
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

        /**
         * @brief Return pointer on HFApp for add/get modules 
         * and take info from Application
         * @return Pointer on HFApp
         */
        HFApp* GetApp() { 
            return this->m_pApp; 
        }

        /**
         * @brief Add plugin in this module
         * @tparam Plugin Type of plugin (derived from HFPlugin)
         * @param args Arguments to use to constructor
         * @return Pointer on created Plugin or nullptr 
         * if plugin already has or allocaate memory failure
         */
        template <typename Plugin, typename... Args, typename = std::enable_if_t<std::is_base_of_v<HFPlugin, Plugin>>>
        Plugin* AddPlugin([[maybe_unused]] Args&&... args) {
            static_assert(is_indexable_v<Plugin>);
            const auto index = type_index<Plugin>::id();

            if(!(index < this->m_Plugins.size())) {
                this->m_Plugins.resize(index + 1);
            }

            if(auto& pPlugin = this->m_Plugins[index]; !pPlugin) {
                pPlugin = HF_NEW Plugin(std::forward<Args>(args)...);
                pPlugin->m_pModule = this;
                this->m_PluginsMap.emplace(HF_CLASSNAME_RT(Plugin), pPlugin);
                return static_cast<Plugin*>(pPlugin);
            }
            return nullptr;
        }

        /**
         * @brief Get plugin from this module, it's extremely cheap.
         * WARNING: to get plugin from third party module you must
         * use method "GetPluginByName" otherwise you get UB
         * @tparam Plugin Type of plugin (derived from HFPlugin)
         * @return Pointer on Plugin or nullptr if plugin not exist
         */
        template <typename Plugin, typename = std::enable_if_t<std::is_base_of_v<HFPlugin, Plugin>>>
        Plugin* GetPlugin() noexcept {
            static_assert(is_indexable_v<Plugin>);
            const auto index = type_index<Plugin>::id();
            if(index < this->m_Plugins.size()) {
                return static_cast<Plugin*>(this->m_Plugins[index]);
            }
            return nullptr;
        }

        /**
         * @brief Get plugin from this or third party module by name
         * @tparam Plugin Type of plugin (derived from HFPlugin)
         * @return Pointer on Plugin or nullptr if plugin not exist
         */
        template <typename Plugin, typename = std::enable_if_t<std::is_base_of_v<HFPlugin, Plugin>>>
        Plugin* GetPluginByName() {
            const auto it = this->m_PluginsMap.find(HF_CLASSNAME_RT(Plugin));
            return it == this->m_PluginsMap.end() ? nullptr : static_cast<Plugin*>(it->second);
        }

        /**
         * @brief Remove plugin from this module. 
         * Don't worry about memory leak, the module will take care 
         * about this and will free memory when the module is free.
         */
        template <typename Plugin, typename = std::enable_if_t<std::is_base_of_v<HFPlugin, Plugin>>>
        void RemovePlugin() noexcept {
            static_assert(is_indexable_v<Plugin>);
            const auto index = type_index<Plugin>::id();
            if(index < this->m_Plugins.size()) {
                this->m_PluginsMap.erase(HF_CLASSNAME_RT(Plugin));
                HF_FREE(this->m_Plugins[index]);
            }
        }

    private:
        HFApp*  m_pApp;

    #if HF_STANDARD_VER <= HF_STANDARD_CPP17
        std::unordered_map<std::string, HFPlugin*> m_PluginsMap;
    #else
        std::unordered_map<std::string, HFPlugin*, HFStringHash, std::equal_to<>> m_PluginsMap;
    #endif

        std::vector<HFPlugin*> m_Plugins;
    };
}

#endif // __COMMON_HFMODULE_HPP__