#include <Include/PluginTestA.hpp>

#include <Module/ModuleTestA/Interface/IPluginTestA.hpp>

#include <Common/ModuleManager.hpp>

namespace Helena
{
	struct MyClass {

	};

	bool PluginTestA::Initialize() {
		std::cout << "[Info ] PluginTestA initialize: " << this->m_pModuleManager->GetAppName() << std::endl;

		// Test Cache pointer for optimization map find
		// Don't worry about storing pointers in Plugin class
		const auto pPluginTestA1 = this->m_pModuleManager->GetPlugin<IPluginTestA>();	// First call GetPlugin for this type use map.find
		const auto pPluginTestA2 = this->m_pModuleManager->GetPlugin<IPluginTestA>();	// Second call GetPlugin for this type use cached pointer

		const auto pPluginSfinae = this->m_pModuleManager->GetPlugin<PluginTestA>();	// return nullptr, not abstract class

		return true;
	}
}