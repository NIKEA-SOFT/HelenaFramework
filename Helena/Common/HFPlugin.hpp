#ifndef __COMMON_HFPLUGIN_HPP__
#define __COMMON_HFPLUGIN_HPP__

#include <vector>
#include <type_traits>
#include "HFPlatform.hpp"

namespace Helena
{
    class HFModule;
    class HFPlugin;
    class HFPluginManager 
    {
        friend class HFModule;
        friend class HFPlugin;

        using PluginID = uint32_t;

        struct type_index_seq { 
            [[nodiscard]] static PluginID next() noexcept {
                static PluginID id {};
                return id++;
            }
        };

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
        virtual ~HFPluginManager() = default;

    protected:
        template <typename Plugin, typename... Args>
        Plugin* AddPlugin([[maybe_unused]] Args&&... args);

        template <typename Plugin>
        Plugin* GetPlugin() noexcept;

        template <typename Plugin>
        void RemovePlugin() noexcept;

    private:
        inline static std::vector<HFPlugin*> m_Plugins;
        inline static HFModule* m_pModule;
    };

    class HFPlugin : public virtual HFPluginManager 
    {
    public:
        virtual ~HFPlugin() = default;

    protected:
        HFModule* GetModule() const {
            return this->m_pModule;     // Get friend pointer
        }
    };

    template <typename Plugin, typename... Args>
    Plugin* HFPluginManager::AddPlugin([[maybe_unused]] Args&&... args) {
        static_assert(std::is_base_of_v<HFPlugin, Plugin>);
        static_assert(is_indexable_v<Plugin>);
        const auto index = type_index<Plugin>::id();

        if(!(index < this->m_Plugins.size())) {
            this->m_Plugins.resize(index + 1);
        }

        if(auto& pPlugin = this->m_Plugins[index]; !pPlugin) {
            pPlugin = HF_NEW Plugin(std::forward<Args>(args)...);
            return static_cast<Plugin*>(pPlugin);
        }
        return nullptr;
    }

    template <typename Plugin>
    Plugin* HFPluginManager::GetPlugin() noexcept {
        static_assert(std::is_base_of_v<HFPlugin, Plugin>);
        static_assert(is_indexable_v<Plugin>);
        const auto index = type_index<Plugin>::id();
        if(index < this->m_Plugins.size()) {
            return static_cast<Plugin*>(this->m_Plugins[index]);
        }
        return nullptr;
    }

    template <typename Plugin>
    void HFPluginManager::RemovePlugin() noexcept {
        static_assert(std::is_base_of_v<HFPlugin, Plugin>);
        static_assert(is_indexable_v<Plugin>);
        const auto index = type_index<Plugin>::id();
        if(index < this->m_Plugins.size()) {
            HF_FREE(this->m_Plugins[index])
        }
    }
}

#endif // __COMMON_HFPLUGIN_HPP__