#include <Include/PluginLog.hpp>
#include <Common/ModuleManager.hpp>

#include <iostream>

namespace Helena
{
	bool PluginLog::Initialize() {
		std::cout << "PluginLog initialize: " << this->m_pModuleManager->GetAppName()  << std::endl;
		return true;
	}
}