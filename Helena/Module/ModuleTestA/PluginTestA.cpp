#include <Include/PluginTestA.hpp>		// Including current plugin header
#include <Common/ModuleManager.hpp>		// Including ModuleManager (need including if you use m_pModuleManager or you need app name or directories)

#include <Module/ModuleTestB/Interface/IPluginTestB.hpp>

namespace Helena
{
	bool PluginTestA::Initialize() {
		std::cout << "[Info ] Initialize " << HF_CLASSNAME(PluginTestA) << ", Serivce: " << this->m_pModuleManager->GetServiceName() << std::endl;

		// Good example: GetPlugin takes an abstract class of Plugin
		// About GetPlugin: This method supports caching pointers inside 
		// the GetPlugin implementation, this frees you from the need 
		// to store pointers to plugins, including third - party modules.
		// It works like this: the first call to GetPlugin for the specified 
		// Type performs a hash map search, if the plugin object exists, 
		// then it is cached. This means that all subsequent calls to GetPlugin 
		// for the same type will no longer call the .find method and perform 
		// a hashmap lookup, but will simply return the cached pointer
		// to the object that was found earlier.
		// This implementation will allow developers to maintain 
		// a clean interface inside plugin classes without losing performance. 
		const auto pPluginTestA1 = this->m_pModuleManager->GetPlugin<IPluginTestA>();	// First call GetPlugin for this type use map.find
		const auto pPluginTestA2 = this->m_pModuleManager->GetPlugin<IPluginTestA>();	// Second call GetPlugin for this type use cached pointer

		// Bad example: GetPlugin only accepts an abstract class of Plugin
		const auto pPluginTestA3 = this->m_pModuleManager->GetPlugin<PluginTestA>();	// Correct: IPluginTestA

		// Get third-module plugin instance and call method
		if(auto pPluginTestB = this->m_pModuleManager->GetPlugin<IPluginTestB>(); pPluginTestB) {
			pPluginTestB->SayHello();
		}

		return true;
	}

	bool PluginTestA::Config() {
		std::cout << "[Info ] Config: " << HF_CLASSNAME(PluginTestA) << ", Path: " << this->m_pModuleManager->GetDirectories()->GetConfigPath() << std::endl;
		return true;
	}

}