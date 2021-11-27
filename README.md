![HelenaFramework](https://user-images.githubusercontent.com/57288440/116739956-00ca2580-a9fd-11eb-9c5d-367f21606456.png)

`HelenaFramework` is a header-only, tiny and easy to use library 
for game backend programming and much more written in **modern C++**.<br/>

---

# Introduction

The HelenaFramework is an architectural pattern used mostly in backend development.

# Features

* User-friendly interface
* High performance
* Modules dll/so with shared memory (across boundary)
* Scalable and flexible
* Cross-platform
* Systems and Events dispatchers
* Header only

## Code Example
```cpp
#include <Helena/Engine/Engine.hpp>

#include <Helena/Systems/EntityComponent.hpp>

// Component
struct UserInfo {
    std::string name;
    std::uint32_t age;
};

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

        auto& ecs = Helena::Engine::GetSystem<Helena::Systems::EntityComponent>();
        ecs.CreateEntity(); // Create entity and trigger event CreateEntity
    }

    void OnEvent(const Helena::Events::Engine::Config&) {
        HELENA_MSG_DEBUG("EngineConfig");
    }

    void OnEvent(const Helena::Events::Engine::Execute&) {
        HELENA_MSG_DEBUG("EngineExecute");
    }

    void OnEvent(const Helena::Events::Engine::Tick& tick) {
        HELENA_MSG_DEBUG("EngineTick: {:.4f}", tick.deltaTime);
    }

    void OnEvent(const Helena::Events::Engine::Update& update) {
        HELENA_MSG_DEBUG("EngineUpdate: {:.4f}", update.deltaTime);
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
        Helena::Engine::RegisterSystem<Helena::Systems::EntityComponent>(); // Entity Component System
        Helena::Engine::RegisterSystem<TestSystem>();                       // Test System
    });

    // Engine loop
    while(Helena::Engine::Heartbeat()) {}

    return 0;
}
```