#include <Include/PluginTestB.hpp>

#include <Module/ModuleLog/Interface/IPluginLog.hpp>

#include <Common/ModuleManager.hpp>

namespace Helena
{
	bool PluginTestB::Initialize() {
		LOG_INFO("Initialize call from {}", HF_CLASSNAME(PluginTestB));
		return true;
	}

	bool PluginTestB::Config() {
		LOG_INFO("Config call from {}", HF_CLASSNAME(PluginTestB));
		return true;
	}

	bool PluginTestB::Execute() {
		LOG_INFO("Execute call from {}", HF_CLASSNAME(PluginTestB));
		return true;
	}

	bool PluginTestB::Update() {
		static auto eventTime = std::chrono::system_clock::now() + std::chrono::seconds(5);
		if(const auto curTime = std::chrono::system_clock::now(); curTime > eventTime) {
			eventTime = curTime + std::chrono::seconds(5);
			LOG_INFO("Update call from {}", HF_CLASSNAME(PluginTestB));
		}
		return true;
	}

	bool PluginTestB::Finalize() {
		LOG_INFO("Finalize call from {}", HF_CLASSNAME(PluginTestB));
		return true;
	}

	void PluginTestB::SayHello() const {
		LOG_INFO("Hello from {}", HF_CLASSNAME(PluginTestB));
	}
}