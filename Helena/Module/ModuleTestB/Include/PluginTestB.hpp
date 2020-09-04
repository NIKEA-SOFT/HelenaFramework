#ifndef MODULE_MODULETESTB_PLUGINTESTB_HPP
#define MODULE_MODULETESTB_PLUGINTESTB_HPP

#include <Interface/IPluginTestB.hpp>

namespace Helena
{
	class ModuleManager;
	class PluginTestB : public IPluginTestB
	{
	public:
		PluginTestB(ModuleManager* pModuleManager)
			: m_pModuleManager(pModuleManager) {}

	public:
		bool Initialize() override;

	private:
		ModuleManager* m_pModuleManager;
	};
}

#endif // MODULE_MODULETESTB_PLUGINTESTB_HPP