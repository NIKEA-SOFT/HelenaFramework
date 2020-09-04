#ifndef __COMMON_HFMODULEMANAGER_H__
#define __COMMON_HFMODULEMANAGER_H__

#include <vector>
#include <type_traits>
#include <unordered_map>

#include "HFPlatform.hpp"

namespace Helena
{
    // forward definition
    class HFApp;
    class HFModule;
    class HFDynLib;

    /*! @brief Manager plugins in dll memory space (friendly interface of class) */
    class HFModuleManager 
    {
        using ModuleID = uint32_t;

        /*! @brief Type index sequencer */
        struct type_index_seq { 
            [[nodiscard]] static ModuleID next() noexcept {
                static ModuleID id {};
                return id++;
            }
        };

        /*! @brief Type index */
        template <typename Type, typename = void>
        struct type_index {
            [[nodiscard]] static ModuleID id() noexcept {
                static const ModuleID id = type_index_seq::next();
                return id;
            }
        };

        template <typename, typename = void>
        struct is_indexable : std::false_type {};

        template <typename Type>
        struct is_indexable<Type, std::void_t<decltype(type_index<Type>::id())>> : std::true_type {};
    
        template <typename Type>
        static constexpr bool is_indexable_v = is_indexable<Type>::value;

        template <typename Module, typename = std::enable_if_t<std::is_base_of_v<HFModule, Module>>>
        static bool AddModule(Module* module) {
            static_assert(is_indexable_v<Module>);

            if(module) {
                const auto index = type_index<Module>::id();
                if(!(index < m_Modules.size())) {
                    m_Modules.resize(index + 1);
                }

                if(auto& pModule = m_Modules[index]; !pModule) {
                    pModule = module;
                    return true;
                }
            }

            HF_ASSERT(false, "Add instance of module failure!");
            return false;
        }

    public:   
        template <typename Base, typename Module, typename = std::enable_if_t<std::is_base_of_v<Base, Module>>>
        static Base* GetModule() noexcept {
            static_assert(is_indexable_v<Module>);
            const auto index = type_index<Module>::id();
            if(index < m_Modules.size()) {
                return static_cast<Base*>(m_Modules[index]);
            }

            HF_ASSERT(false, "Get instance of module failure!");
            return nullptr;
        }


        inline static std::vector<HFModule*> m_Modules;
        inline static HFApp* m_pApp = nullptr;
    };
}

#endif // __COMMON_HFMODULEMANAGER_H__