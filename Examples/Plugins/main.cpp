#include <Systems/PluginManager.hpp>

// WARNING: For using this example you need "PluginManager" system
// PluginManager - is a System that will allow you to load dynamic libraries at runtime.
// The term System implies that this is a class that brings its own logic when you register it through the framework

// The code below is just an example of how we could implement an AudioLogger that listens for events from the AudioSystem,
// but still have architecturally correct code. The current plugin implements the AudioLogger class (system)
// that listens for an event from the AudioSystem, but the AudioSystem is not related to the AudioLogger in any way.
// This allows us to write code with the least possible dependencies between them.

// For example, we have AudioSystem (class - system for playing audio)
// Imagine that this class provides its own signal, which we will listen to in the future.
namespace Events::AudioSystem
{
	struct Play {
		std::string m_AudioName;
		std::uint32_t m_Volume;
		std::uint32_t m_Duration;
	};
}

// Small example class (system)
// For example, we'd like to log which sounds the AudioSystem play.
// We can do this without changing the AudioSystem and without recompiling the code.
class AudioLogger
{
public:
	AudioLogger() {
		// Start listening AudioSystem event
		Helena::Engine::SubscribeEvent<Events::AudioSystem::Play>(&AudioLogger::OnAudioPlay);

		// The current listener is only used for hot reload testing
		//Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>(&AudioLogger::OnInit);
	}

	~AudioLogger() {
		Helena::Engine::UnsubscribeEvent<Events::AudioSystem::Play>(&AudioLogger::OnAudioPlay);
		//Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Init>(&AudioLogger::OnInit);
	}

	// Serialize data when reloading
	static void SerializeState() {
		// serialize current class states and members
		/* TODO
		Helena::Types::FileDatabase fileDB{"./cache/AudioLogger.hdb"};
		fileDB << m_FieldA;
		fileDB << m_FieldB;
		fileDB << m_FieldC;
		*/
	}

	// Deserialize data when current plugin Load
	static void DeserializeState() {
		// deserialize current class states and members
		/* TODO
		Helena::Types::FileDatabase fileDB{"./cache/AudioLogger.hdb"};
		if(fileDB.Has()) {
			fileDB >> m_FieldA;
			fileDB >> m_FieldB;
			fileDB >> m_FieldC;
		}
		fileDB.Delete();
		*/
	}

	//void OnInit() {
	//	HELENA_MSG_INFO("Hello from AudioLogger init callback");
	//}

private:
	// Callback for logging AudioSystem event's
	void OnAudioPlay(const Events::AudioSystem::Play& event) {
		HELENA_MSG_INFO("AudioSystem now play sound: {}, with volume: {} and duration: {}", event.m_AudioName, event.m_Volume, event.m_Duration);
	}

	std::uint32_t m_FieldA;
	std::string m_FieldB;
	std::vector<std::uint32_t> m_FieldC;
};

// EntryPoint (new)
HELENA_PLUGIN_API void PluginMain(Helena::Systems::PluginManager::EState state, Helena::Engine::Context& ctx)
{
	switch(state)
	{
		case Helena::Systems::PluginManager::EState::Load: {
			HELENA_MSG_NOTICE("AudioLogger: Init");
			Helena::Engine::Initialize(ctx);
			Helena::Engine::RegisterSystem<AudioLogger>();
		} break;

		// Support hot reloading
		case Helena::Systems::PluginManager::EState::Reload: {
			HELENA_MSG_NOTICE("AudioLogger: Reload");
		} break;

		case Helena::Systems::PluginManager::EState::Unload: {
			HELENA_MSG_NOTICE("AudioLogger: Unload");

			Helena::Engine::RemoveSystem<AudioLogger>();
		} break;
	};
}