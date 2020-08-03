#ifndef COMMON_HFDYNPLUGIN_HPP
#define COMMON_HFDYNPLUGIN_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

#include "HFModule.hpp"

namespace Helena
{
    enum class HF_MODULE_STATE : uint8_t 
    {
        HF_MODULE_INIT,
        //HF_MODULE_RELOAD,
        HF_MODULE_FREE,
    };
    
    class HFApp;
    class HFDynLib final
    {
        friend class HFApp;
        using fnRegisterModule = bool (*)(HFApp*, HF_MODULE_STATE);

    public:
        explicit HFDynLib(std::string_view name) : m_Name(name), m_pModule(nullptr), m_pHandle(nullptr), m_State(HF_MODULE_STATE::HF_MODULE_INIT) {}
        
    public:
        bool Load(HFApp* pApp) 
        {
            // Check module instance
            this->m_pHandle = static_cast<HF_MODULE_HANDLE>(HF_MODULE_LOAD(this->m_Name.data()));
            if(!this->m_pHandle) {
                std::cerr << "[Error] Load module: \"" << this->m_Name << "\" failure!" << std::endl;
                return false;
            } 

            // Check module main callback
            const auto pMain = reinterpret_cast<fnRegisterModule>(HF_MODULE_GETSYM(this->m_pHandle, HF_MODULE_CALLBACK));
            if(!pMain) {
                std::cerr << "[Error] Load module: \"" << this->m_Name << "\", API: \"" << HF_MODULE_CALLBACK << "\" not found!" << std::endl;
                return false;
            }
            
            // Call module main callback
            this->m_State = HF_MODULE_STATE::HF_MODULE_INIT;
            if(!pMain(pApp, this->m_State)) {
                std::cerr << "[Error] Load module: \"" << this->m_Name << "\", API: \"" << HF_MODULE_CALLBACK << "\" return error!" << std::endl;
                return false;
            }

            return true;
        }

        void Unload(HFApp* pApp) 
        {
            this->m_State = HF_MODULE_STATE::HF_MODULE_FREE;
            if(!this->m_pHandle) {
                std::cerr << "[Error] Unload module: \"" << this->m_Name << "\", failure: m_pHandle is nullptr" << std::endl;
                return;
            }
            
            if(const auto pMain = reinterpret_cast<fnRegisterModule>(HF_MODULE_GETSYM(this->m_pHandle, HF_MODULE_CALLBACK)); pMain) {
                (void)pMain(pApp, this->m_State);
            }

            HF_FREE(this->m_pModule);
            HF_MODULE_UNLOAD(this->m_pHandle);
        }

    private:
        std::string_view    m_Name;
        HFModule*           m_pModule;
        HF_MODULE_HANDLE    m_pHandle;
        HF_MODULE_STATE     m_State;
    };
}

#endif