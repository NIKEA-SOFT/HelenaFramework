#ifndef MODULE_MODULETESTA_PLUGINTESTA_HPP
#define MODULE_MODULETESTA_PLUGINTESTA_HPP

#include <Interface/IPluginTestA.hpp>

namespace Helena
{
	class ModuleManager;
	class PluginTestA : public IPluginTestA
	{
	protected:
		bool Initialize() override;
		bool Config() override;

	public:
		PluginTestA(ModuleManager* pModuleManager) : m_pModuleManager(pModuleManager) {}

	private:
		ModuleManager* m_pModuleManager;
	};
}

#endif // MODULE_MODULETESTA_PLUGINTESTA_HPP