#include <Include/PluginTestB.hpp>
#include <Common/ModuleManager.hpp>

#include <iostream>

namespace Helena
{
	bool PluginTestB::Initialize() {
		std::cout << "[Info ] Initialize " << HF_CLASSNAME(PluginTestB) << ", Serivce: " << this->m_pModuleManager->GetAppName() << std::endl;
		return true;
	}

	void PluginTestB::SayHello() const {
		std::cout << "[Info ] Hello from " << HF_CLASSNAME(PluginTestB) << std::endl;
	}
}