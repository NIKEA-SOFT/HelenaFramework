#ifndef COMMON_HFPLUGININFO_HPP
#define COMMON_HFPLUGININFO_HPP

#include "HFPlatform.hpp"

namespace Helena
{
	template <typename Module>
    class HFPluginBase
	{
    public:
	    HFPluginBase() : m_pModule(nullptr) {}
		virtual ~HFPluginBase() = default;

		[[nodiscard]] Module* GetModule() const {
		    return this->m_pModule;
	    }
		
    private:
        Module* m_pModule;
    };
	
    class HFPlugin : public HFPluginBase<HFPlugin>
    {    	
    public:
        explicit HFPlugin() = default;
        virtual ~HFPlugin() = default;
        HFPlugin(const HFPlugin&) = delete;
        HFPlugin(HFPlugin&&) = delete;
        HFPlugin& operator=(const HFPlugin&) = delete;
        HFPlugin& operator=(HFPlugin&&) = delete;


    private:
		//HFModule* m_pModule;
    };
}

#endif