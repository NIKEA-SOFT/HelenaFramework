#include <Helena/Engine/Engine.hpp>

#include <Helena/Systems/EntityComponent.hpp>
#include <Helena/Systems/ResourceManager.hpp>
#include <Helena/Systems/PluginManager.hpp>

// Component
struct UserInfo {
    std::string name;
    std::uint32_t age;
};

// Declaration class from HelenaPlugin;
// Here should be include...
class GameApplication;

// Test System
class TestSystem
{
public:
    TestSystem() {
        // Start listen Engine events
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>    (&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Config>  (&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Execute> (&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Tick>    (&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Update>  (&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Finalize>(&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Shutdown>(&TestSystem::OnEvent);

        // Start listen events from system EntityComponent
        Helena::Engine::SubscribeEvent<Helena::Events::EntityComponent::CreateEntity>(&TestSystem::OnCreateEntity);
        Helena::Engine::SubscribeEvent<Helena::Events::EntityComponent::RemoveEntity>(&TestSystem::OnRemoveEntity);
    }
    ~TestSystem() = default;

    void OnEvent(const Helena::Events::Engine::Init&) {
        HELENA_MSG_DEBUG("EventInit");

        auto [ecs, pluginManager] = Helena::Engine::GetSystem<Helena::Systems::EntityComponent, Helena::Systems::PluginManager>();
        ecs.CreateEntity(); // Create entity and trigger event CreateEntity

        if(pluginManager.Create("HelenaPlugin.dll")) {
            HELENA_MSG_DEBUG("Plugin loaded!");
        }

        // here used reinterpret_cast because i'm not including header for GameApplication
        // you should include it in your project for using class members and functions!
        auto& temp = Helena::Engine::GetSystem<GameApplication>();
        HELENA_MSG_INFO("GameApplication value: {}", *reinterpret_cast<std::uint32_t*>(&temp));
    }

    void OnEvent(const Helena::Events::Engine::Config&) {
        HELENA_MSG_DEBUG("EngineConfig");
    }

    void OnEvent(const Helena::Events::Engine::Execute&) {
        HELENA_MSG_DEBUG("EngineExecute");
    }

    void OnEvent(const Helena::Events::Engine::Tick& event) {
        HELENA_MSG_DEBUG("EngineTick: {:.4f}", event.deltaTime);
    }

    void OnEvent(const Helena::Events::Engine::Update& event) {
        HELENA_MSG_DEBUG("EngineUpdate: {:.4f}", event.fixedTime);
    }

    void OnEvent(const Helena::Events::Engine::Finalize&) {
        HELENA_MSG_DEBUG("EngineFinalize");
    }

    void OnEvent(const Helena::Events::Engine::Shutdown&) {
        HELENA_MSG_DEBUG("EngineShutdown");
    }

    void OnCreateEntity(const Helena::Events::EntityComponent::CreateEntity& event) {
        auto& ecs = Helena::Engine::GetSystem<Helena::Systems::EntityComponent>();
        auto& userInfo = ecs.AddComponent<UserInfo>(event.Entity, "Helena", 30u);

        HELENA_MSG_DEBUG("Entity created, user name: {}, age: {}", userInfo.name, userInfo.age);
        ecs.RemoveEntity(event.Entity); // Removed entity after trigger RemoveEntity event
    }

    void OnRemoveEntity(const Helena::Events::EntityComponent::RemoveEntity& event) {
        auto& ecs = Helena::Engine::GetSystem<Helena::Systems::EntityComponent>();
        auto& userInfo = ecs.GetComponent<UserInfo>(event.Entity);

        HELENA_MSG_DEBUG("Entity: {} removed!", userInfo.name);
    }
};

int main(int argc, char** argv)
{
    // Engine started from Initialize method
    Helena::Engine::Context::Initialize();                          // Initialize Context (Context used in Engine)
    Helena::Engine::Context::SetAppName("Test Framework");          // Set application name
    Helena::Engine::Context::SetTickrate(60.f);                     // Set Update tickrate
    Helena::Engine::Context::SetCallback([]()                       // Register systems happen in this callback
    {
        // Register all used systems
        Helena::Engine::RegisterSystem<Helena::Systems::ResourceManager>(); // Resource storage System
        Helena::Engine::RegisterSystem<Helena::Systems::EntityComponent>(); // Entity Component System
        Helena::Engine::RegisterSystem<Helena::Systems::PluginManager>();   // Plugin manager System
        Helena::Engine::RegisterSystem<TestSystem>();                       // Test System
    });

    // Engine loop
    while(Helena::Engine::Heartbeat()) {}

    return 0;
}