#ifndef __COMMON_HFMODULE_HPP__
#define __COMMON_HFMODULE_HPP__

#include <iostream>
#include <unordered_map>
#include <string>

#include "HFPlugin.hpp"

namespace Helena
{
    class HFApp;
    class HFPlugin;

    class HFModule : public virtual HFPluginManager
    {
        friend class HFApp;

    public:
        explicit HFModule() : m_pApp(nullptr) {}
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

    private:
        void SetAppByFriend(HFApp* pApp) {
            this->m_pApp = pApp;
            this->m_pModule = this;
        }

    private:
        HFApp*  m_pApp;
    };
}

#endif // __COMMON_HFMODULE_HPP__