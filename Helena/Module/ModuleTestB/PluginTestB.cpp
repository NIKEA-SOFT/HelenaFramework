#include <Include/PluginTestB.hpp>
#include <Common/ModuleManager.hpp>

#include <iostream>

namespace Helena
{
	bool PluginTestB::Initialize() {
		std::cout << "PluginTestA initialize: " << this->m_pModuleManager->GetAppName() << std::endl;
		return true;
	}
}