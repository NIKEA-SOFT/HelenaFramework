#ifndef __COMMON_HFAPP_HPP__
#define __COMMON_HFAPP_HPP__

#include <thread>
#include <chrono>

#include "HFSingleton.hpp"
#include "HFDynLib.hpp"
#include "HFArgs.hpp"
#include "HFMeta.hpp"
#include "HFXml.hpp"
#include "HFUtil.hpp"

// TODO: Remove args library (reason: clogging binary files)

namespace Helena
{

	// constexpr const char* spdlog_filename(std::string_view filename) {
	// 	constexpr char symbols[]{'\\', '/'};
	// 	const auto it = std::find_first_of(filename.rbegin(), filename.rend(), std::begin(symbols), std::end(symbols));
	// 	return it == filename.rend() ? filename.data() : &(*std::prev(it));
	// }

    // // Assembly "if" optimization
	// inline decltype(auto) g_LoggerRef = spdlog::details::registry::instance();

	// #define LOG_TRACE(my_fmt, ...)		g_LoggerRef.get_default_raw()->log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, nullptr}, spdlog::level::trace,		my_fmt, ##__VA_ARGS__);
	// #define LOG_DEBUG(my_fmt, ...)		g_LoggerRef.get_default_raw()->log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, nullptr}, spdlog::level::debug,		my_fmt,	##__VA_ARGS__);
	// #define LOG_INFO(my_fmt, ...)		g_LoggerRef.get_default_raw()->log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, nullptr}, spdlog::level::info,		my_fmt,	##__VA_ARGS__);
	// #define LOG_WARN(my_fmt, ...)		g_LoggerRef.get_default_raw()->log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, nullptr}, spdlog::level::warn,		my_fmt,	##__VA_ARGS__);
	// #define LOG_ERROR(my_fmt, ...)		g_LoggerRef.get_default_raw()->log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, nullptr}, spdlog::level::err,		my_fmt, ##__VA_ARGS__);
	// #define LOG_CRITICAL(my_fmt, ...)	g_LoggerRef.get_default_raw()->log(spdlog::source_loc{spdlog_filename(__FILE__), __LINE__, nullptr}, spdlog::level::critical,	my_fmt, ##__VA_ARGS__);    

    class HFApp final : public HFSingleton<HFApp>
    {
        friend int HelenaFramework(int, char**);

    private:
        /**
         * @brief Initialize Helena Framework
         * 
         * @param argc Number of argumments
         * @param argv Pointer on arguments
         * @return false if initialize Helena failure
         */
        bool Initialize(const int argc, const char* const* argv) 
        { 
            HFArgs::Parser  argsParser("Hello, Helena!", "Good luck Helena!");
            HFArgs::Group   argsGroup(argsParser, "Helena flags:", HFArgs::Group::Validators::All);
            HFArgs::ValueFlag<std::string> argsFlag1(argsGroup, "name", "App name", {"app"});
            HFArgs::ValueFlag<std::string> argsFlag2(argsGroup, "path", "Config files path", {"config-dir"});
            HFArgs::ValueFlag<std::string> argsFlag3(argsGroup, "path", "Module files path", {"module-dir"});

            std::vector<int> vec = {12, 10, 33};
            for(auto val : vec) {
                std::cout << val << std::endl;
            }

            try {
                argsParser.ParseCLI(argc, argv);
            } catch (HFArgs::Help) {
                std::cerr << argsParser << std::endl;
                return true;
            } catch (HFArgs::Error& err) {
                std::cerr << argsParser << std::endl;
                std::cerr << "[Error] ArgsParse failure: " << err.what() << std::endl;
                return false;
            }

            this->m_Name = argsFlag1.Get();
            this->m_ConfigPath = argsFlag2.Get();
            this->m_ModulePath = argsFlag3.Get();

            std::cout << "[Info] App name=" << argsFlag1.Get() << std::endl;
            std::cout << "[Info] Config dir=" << argsFlag2.Get() << std::endl;
            std::cout << "[Info] Module dir=" << argsFlag3.Get() << std::endl;

            if(!this->Configuration()) {
                std::cerr << "[Error] Configuration Application failure!" << std::endl;
                return false;
            } else std::cout << "[Info] Configuration Application success!" << std::endl;

            if(!this->AppInitModule()) {
                std::cerr << "[Error] Initialize modules failure!" << std::endl;
                return false;
            } else std::cout << "[Info] Initialize modules success!" << std::endl;

            if(!this->AppInit()) {
                std::cerr << "[Error] AppInit failure!" << std::endl;
                return false;
            } else std::cout << "[Info] AppInit success!" << std::endl;

            if(!this->AppConfig()) {
                std::cerr << "[Error] AppConfig failure!" << std::endl;
                return false;
            } else std::cout << "[Info] AppConfig success!" << std::endl;

            if(!this->AppStart()) {
                std::cerr << "[Error] AppStart failure!" << std::endl;
                return false;
            } else std::cout << "[Info] AppStart success!" << std::endl;

            if(!this->AppUpdate()) {
                std::cerr << "[Error] AppUpdate failure!" << std::endl;
                return false;
            } else std::cout << "[Info] AppUpdate success!" << std::endl;

            if(!this->AppShut()) {
                std::cerr << "[Error] AppShut failure!" << std::endl;
                return false;
            } else std::cout << "[Info] AppShut success!" << std::endl;

            this->Finalize();
            return true;
        }

        /*! @brief Finalize Helena Framework */
        void Finalize() 
        {
            for(auto& pDynLib : this->m_DynLibs) {
                pDynLib->Unload(this);
                HF_FREE(pDynLib);
            }
            this->m_DynLibs.clear();
        }
    
    public:

        /**
         * @brief Add module in HFApp, use RemoveModule for free allocated memory
         * @tparam Module Type of module (derived from HFModule)
         * @param args Arguments to use to constructor
         */       
        template <typename Module, typename... Args, typename = std::enable_if_t<std::is_base_of_v<HFModule, Module>>>
        void AddModule([[maybe_unused]] Args&&... args)
        {
            const auto pDynLib = this->m_DynLibs.back();
            if(pDynLib->m_pModule) {
                std::cerr 
                    << "[Error] Module: \"" 
                    << pDynLib->m_Name 
                    << "\", error: only one class per module!" 
                    << std::endl;
                return;               
            }

            if(const auto it = this->m_Modules.find(HF_CLASSNAME_RT(Module)); it != this->m_Modules.end()) {
                std::cerr 
                    << "[Error] Module: \"" 
                    << pDynLib->m_Name 
                    << "\", error: this class registered from other module: \""
                    << it->second->m_Name 
                    << "\"" 
                    << std::endl;
                return;
            }

            if(pDynLib->m_pModule = HF_NEW Module(std::forward<Args>(args)...); !pDynLib->m_pModule) {
                std::cerr 
                    << "[Error] Module: \"" 
                    << pDynLib->m_Name 
                    << "\", error: allocate memory!" 
                    << std::endl;
                return;
            }

            if(const auto [it, bRes] = this->m_Modules.emplace(HF_CLASSNAME_RT(Module), pDynLib); !bRes) {
                std::cerr 
                    << "[Error] Module: \"" 
                    << pDynLib->m_Name 
                    << "\", error: allocate memory for map!" 
                    << std::endl;
                HF_FREE(pDynLib->m_pModule);
                return;               
            }
            
            pDynLib->m_Version = HF_COMPILER;
            pDynLib->m_pModule->m_pApp = this;
        }

        /**
         * @brief Get module from HFApp
         * @tparam Module Type derived from HFModule
         * @return Pointer on Module or nullptr if type of module not found
         */
        template <typename Module, typename = std::enable_if_t<std::is_base_of_v<HFModule, Module>>>
        Module* GetModule() noexcept {
            const auto it = this->m_Modules.find(HF_CLASSNAME_RT(Module));
            return it == this->m_Modules.end() ? nullptr : static_cast<Module*>(it->second->m_pModule);
        }

        /**
         * @brief Remove module from HFApp, its correctly free allocated memory from this module
         * @tparam Module Type derived from HFModule
         */
        template <typename Module, typename = std::enable_if_t<std::is_base_of_v<HFModule, Module>>>
        void RemoveModule() noexcept {
            if(const auto it = this->m_Modules.find(HF_CLASSNAME_RT(Module)); it != this->m_Modules.end()) {
                HF_FREE(it->second->m_pModule);
                this->m_Modules.erase(it);
            }
        }

    private:

        /**
         * @brief Initialize Helena Modules from config
         * @return false if same module EP not found, export function not found or callback return false
         */
        bool AppInitModule() 
        {
            for(std::string_view moduleName : this->m_ModulesConfig) {
                if(const auto& pDynLib = this->m_DynLibs.emplace_back(HF_NEW HFDynLib(moduleName)); !pDynLib->Load(this)) {
                    return false;
                }
            }

            // Check version of compiler on all modules
            if(!this->m_Modules.empty()) {
                const auto version = this->m_Modules.begin()->second->m_Version;
                for(const auto& data : this->m_Modules) {
                    const auto module = data.second;
                    if(module->m_Version != version) {
                        std::cerr << "[Error] Module: \"" << module->m_Name << "\", compiler version is different!" << std::endl;
                        return false;
                    }
                }
            }
            return true;
        }

        /**
         * @brief Call virtual AppInit in all modules
         * @return False if one of the module return false
         */
        bool AppInit() 
        {
            for(const auto& pDynLib : this->m_DynLibs) {
                if(!pDynLib->m_pModule->AppInit()) {
                    return false;
                }
            }
            return true;
        }

        /**
         * @brief Call virtual AppConfig in all modules
         * @return False if one of the module return false
         */
        bool AppConfig() 
        {
            for(const auto& pDynLib : this->m_DynLibs) {
                if(!pDynLib->m_pModule->AppConfig()) {
                    return false;
                }
            }
            return true;
        }

        /**
         * @brief Call virtual AppStart in all modules
         * @return False if one of the module return false
         */       
        bool AppStart() 
        {
            for(const auto& pDynLib : this->m_DynLibs) {
                if(!pDynLib->m_pModule->AppStart()) {
                    return false;
                }
            }
            return true;
        }

        /**
         * @brief Call virtual AppUpdate in all modules (main app loop)
         * @return False if one of the module return false
         */
        bool AppUpdate() 
        {
            while(!this->m_bFinish)
            {
                for(const auto& pDynLib : this->m_DynLibs) {
                    if(!pDynLib->m_pModule->AppUpdate()) {
                        return false;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            return true;
        }

        /**
         * @brief Call virtual AppShut in all modules (main app loop)
         * @return False if one of the module return false
         */
        bool AppShut() 
        {
            for(const auto& pDynLib : this->m_DynLibs) 
            {
                if(!pDynLib->m_pModule->AppShut()) {
                    return false;
                }
            }
            return true;
        }

        /**
         * @brief Read config files before init
         * @return False if load/parse config file failure
         */
        bool Configuration() 
        {
            pugi::xml_document doc;
            const std::string config(this->m_ConfigPath + HF_SEPARATOR + Meta::Helena::GetConfig().data());
            const pugi::xml_parse_result parseResult = doc.load_file(config.c_str(), pugi::parse_default | pugi::parse_comments);

            if(!parseResult) {
                std::cout << "[Error] Configuration failure, config: \"" << config << "\", error: " << parseResult.description() << std::endl;
                return false;
            }

            // Applications 
            const auto node = doc.child(Meta::Helena::GetApplications().data());
            if(node.empty()) {
                std::cout 
                << "[Error] Config: \"" << config 
                << "\", Node: \"" << Meta::Helena::GetApplications()
                << "\" not found!" << std::endl;
                return false;               
            }

            const auto node_app = node.find_child_by_attribute(Meta::Helena::GetName().data(), this->m_Name.c_str());
            if(node_app.empty()) {
                std::cout << "[Error] Config: \"" << config << "\", Node: \"" << Meta::Helena::GetApp() << "\", App Name: \"" << this->m_Name << "\" not found!" << std::endl;
                return false;
            }
            
            // Get node
            const auto node_module = node_app.child(Meta::Helena::GetModule().data());
            if(node_module.empty()) {
                std::cout 
                << "[Error] Config: \"" << config 
                << "\", App Name: \"" << this->m_Name 
                << "\", Node: \"" << Meta::Helena::GetModule() 
                << "\", node not found!" << std::endl;
                return false;                  
            }

            // Get attribute from node
            std::string_view module_name = node_module.attribute(Meta::Helena::GetName().data()).as_string(); 
            if(module_name.empty()) {
                
                std::cout 
                << "[Error] Config: \"" << config 
                << "\", App Name: \"" << this->m_Name 
                << "\", Node: \"" << Meta::Helena::GetModule() 
                << "\", module name is empty!" << std::endl;
                return false;
            }
            
            this->m_ModulesConfig = HFUtil::Split<std::string>(module_name, ",", true); // copy ellision used
            return true;
        }

    private:
        std::unordered_map<std::string, HFDynLib*> m_Modules;
        std::vector<HFDynLib*> m_DynLibs;
        std::vector<std::string> m_ModulesConfig;

        std::string m_Name;
        std::string m_ConfigPath;
        std::string m_ModulePath;

        bool m_bFinish;
    };

    inline int HelenaFramework(int argc, char** argv) 
    {
        if(!HFApp::GetInstance().Initialize(argc, argv)) {
            std::cerr << "[Error] Inititalize HelenaFramework failure!" << std::endl;
            HFApp::GetInstance().Finalize();
            return 1;
        }
        
        return 0;
    }
}

#endif // __COMMON_HFAPP_HPP__