#include <Include/PluginTestA.hpp>		// Including current plugin header

#include <Common/Service.hpp>

#include <Module/ModuleLog/Interface/IPluginLog.hpp>		// Including other module plugin interface
#include <Module/ModuleTestB/Interface/IPluginTestB.hpp>	// Including other module plugin interface

namespace Helena
{
	bool PluginTestA::Initialize() {
		LOG_INFO("Initialize call from {}", HF_CLASSNAME(PluginTestA));
		
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
		const auto pluginTestA1 = GetService()->GetModuleManager()->GetPlugin<IPluginTestA>();	// First call GetPlugin for this type use map.find
		const auto pluginTestA2 = GetService()->GetModuleManager()->GetPlugin<IPluginTestA>();	// Second call GetPlugin for this type use cached pointer

		// Get third-module plugin instance and call method
		if(const auto pluginTestB = GetService()->GetModuleManager()->GetPlugin<IPluginTestB>(); pluginTestB) {
			pluginTestB->SayHello();
		}

		// Log testing, this macros use pointer m_pModuleManager->GetPlugin<IPluginLog>
		LOG_TRACE("Logger test from {}", HF_CLASSNAME(PluginTestA));
		LOG_DEBUG("Logger test from {}", HF_CLASSNAME(PluginTestA));
		LOG_INFO("Logger test from {}",	HF_CLASSNAME(PluginTestA));
		LOG_WARN("Logger test from {}", HF_CLASSNAME(PluginTestA));
		LOG_ERROR("Logger test from {}", HF_CLASSNAME(PluginTestA));
		LOG_CRITICAL("Logger test from {}", HF_CLASSNAME(PluginTestA));

		return true;
	}

	bool PluginTestA::Config() {
		LOG_INFO("Config call from {}", HF_CLASSNAME(PluginTestA));
		return true;
	}

	bool PluginTestA::Execute() {
		LOG_INFO("Execute call from {}", HF_CLASSNAME(PluginTestA));
		return true;
	}

	bool PluginTestA::Update() {
		static auto eventTime = std::chrono::system_clock::now() + std::chrono::seconds(5);
		if(const auto curTime = std::chrono::system_clock::now(); curTime > eventTime) {
			eventTime = curTime + std::chrono::seconds(5);
			LOG_INFO("Update call from {}", HF_CLASSNAME(PluginTestA));
		}
		return true;
	}

	bool PluginTestA::Finalize() {
		LOG_INFO("Finalize call from {}", HF_CLASSNAME(PluginTestA));
		return true;
	}
}