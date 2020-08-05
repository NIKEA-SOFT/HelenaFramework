#ifndef COMMON_HFDYNPLUGIN_HPP
#define COMMON_HFDYNPLUGIN_HPP

#include "HFModule.hpp"

namespace Helena
{
    enum class HF_MODULE_STATE : uint8_t 
    {
        HF_MODULE_INIT,
        HF_MODULE_FREE
    };
    
    class HFDynLib final
    {
        friend class HFApp;
        using HFMain = void (*)(HFApp*, HF_MODULE_STATE);

    public:
        HFDynLib(std::string_view name, HFApp* pApp) 
        : m_Name(name)
        , m_pApp(pApp)
        , m_pModule(nullptr)
        , m_pHandle(nullptr)
        , m_State(HF_MODULE_STATE::HF_MODULE_INIT)
        , m_Version(0u) {}
    public:
        bool Load() 
        {
            if(this->m_pHandle = static_cast<HF_MODULE_HANDLE>(HF_MODULE_LOAD(this->m_Name.data())); this->m_pHandle) {
                if(const auto pMain = this->GetEntryPoint(); pMain) {
                    pMain(this->m_pApp, this->m_State);
                    if(this->m_pModule) { 
                        std::cout << "[Info] Module: \"" << this->m_Name << "\" loaded!" << std::endl;
                        return true;
                    }
                    std::cerr << "[Error] Load module: \"" << this->m_Name << "\", API: \"" << HF_MODULE_CALLBACK << "\" did not add class!" << std::endl;
                } else { std::cerr << "[Error] Load module: \"" << this->m_Name << "\", API: \"" << HF_MODULE_CALLBACK << "\" not found!" << std::endl; }
            } else { std::cerr << "[Error] Load module: \"" << this->m_Name << "\" failure!" << std::endl; }
            return false;
        }

        void Unload() 
        {
            this->m_State = HF_MODULE_STATE::HF_MODULE_FREE;
            if(this->m_pHandle) 
            {
                if(const auto pMain = this->GetEntryPoint(); pMain) {
                    pMain(this->m_pApp, this->m_State);
                }

                if(this->m_pModule) {
                    std::cerr << "[Warn] Module: \"" << this->m_Name << "\", warn: forgot HF_FREE object class from module before unload!" << std::endl;
                }

                HF_MODULE_UNLOAD(this->m_pHandle);
                std::cout << "[Info] Module: \"" << this->m_Name << "\" unloaded!" << std::endl;
            }
        }

    public:
        std::string_view GetName() const {
            return this->m_Name;
        }

        HFModule* GetModule() const {
            return this->m_pModule;
        }

        HFMain GetEntryPoint() const {
            return reinterpret_cast<HFMain>(HF_MODULE_GETSYM(this->m_pHandle, HF_MODULE_CALLBACK));
        }

        HF_MODULE_STATE GetState() const {
            return this->m_State;
        }

        uint8_t GetVersion() const {
            return this->m_Version;
        }

    private:
        std::string_view    m_Name;
        HFApp*              m_pApp;
        HFModule*           m_pModule;
        HF_MODULE_HANDLE    m_pHandle;
        HF_MODULE_STATE     m_State;
        uint8_t             m_Version;
    };
}

#endif