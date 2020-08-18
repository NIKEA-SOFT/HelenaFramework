#ifndef __COMMON_HFDYNPLUGIN_HPP__
#define __COMMON_HFDYNPLUGIN_HPP__

#include "HFModule.hpp"

namespace Helena
{
    /*! @brief Module states */
    enum class HF_MODULE_STATE : uint8_t 
    {
        HF_MODULE_INIT,
        HF_MODULE_FREE
    };
    

    /*! @brief Manages the life of loaded modules */
    class HFDynLib final
    {
        using HFMain = void (*)(HFApp*, HF_MODULE_STATE);

    public:
        HFDynLib(std::string_view name) 
        : m_Name(name)
        , m_pModule(nullptr)
        , m_pHandle(nullptr)
        , m_Version(0u)
        , m_State(HF_MODULE_STATE::HF_MODULE_INIT) {}

    public:
        void SetModule(HFModule* pModule) {
            this->m_pModule = pModule;
        }

        void SetVersion(uint8_t version) {
            this->m_Version = version;
        }

        void SetState(HF_MODULE_STATE state) {
            this->m_State = state;
        }
        
        auto GetName() const {
            return this->m_Name;
        }

        auto& GetModule() {
            return this->m_pModule;
        }

        auto GetVersion() const {
            return this->m_Version;
        }

        auto GetState() const {
            return this->m_State;
        }

        /**
         * @brief Load module and call EP
         * @param pApp Pointer on HFApp
         * @return False if load module failure
         */
        bool Load(HFApp* pApp) 
        {
            if(this->m_pHandle = static_cast<HF_MODULE_HANDLE>(HF_MODULE_LOAD(this->m_Name.data())); this->m_pHandle) {
                if(const auto pMain = reinterpret_cast<HFMain>(HF_MODULE_GETSYM(this->m_pHandle, HF_MODULE_CALLBACK)); pMain) {
                    pMain(pApp, this->m_State);
                    if(this->m_pModule) { 
                        std::cout << "[Info] Module: \"" << this->m_Name << "\" loaded!" << std::endl;
                        return true;
                    }
                    std::cerr << "[Error] Load module: \"" << this->m_Name << "\", API: \"" << HF_MODULE_CALLBACK << "\" did not add class!" << std::endl;
                } else { std::cerr << "[Error] Load module: \"" << this->m_Name << "\", API: \"" << HF_MODULE_CALLBACK << "\" not found!" << std::endl; }
            } else { std::cerr << "[Error] Load module: \"" << this->m_Name << "\" failure!" << std::endl; }
            return false;
        }

        /**
         * @brief Call EP and unload module
         * @param pApp Pointer on HFApp
         */
        void Unload(HFApp* pApp) 
        {
            this->m_State = HF_MODULE_STATE::HF_MODULE_FREE;
            if(this->m_pHandle) 
            {
                if(const auto pMain = reinterpret_cast<HFMain>(HF_MODULE_GETSYM(this->m_pHandle, HF_MODULE_CALLBACK)); pMain) {
                    pMain(pApp, this->m_State);
                }

                if(this->m_pModule) {
                    std::cerr << "[Warn] Module: \"" << this->m_Name << "\", warn: forgot HF_FREE object class from module before unload!" << std::endl;
                }

                HF_MODULE_UNLOAD(this->m_pHandle);
                std::cout << "[Info] Module: \"" << this->m_Name << "\" unloaded!" << std::endl;
            }
        }
        
    private:
        std::string_view    m_Name;
        HFModule*           m_pModule;
        HF_MODULE_HANDLE    m_pHandle;
        uint8_t             m_Version;
        HF_MODULE_STATE     m_State;
    };
}

#endif // __COMMON_HFDYNPLUGIN_HPP__