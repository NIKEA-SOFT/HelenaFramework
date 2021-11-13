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
        Helena::Engine::SubscribeEvent<Helena::Events::EngineInit>(&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::EngineConfig>(&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::EngineExecute>(&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::EngineTick>(&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::EngineUpdate>(&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::EngineFinalize>(&TestSystem::OnEvent);
        Helena::Engine::SubscribeEvent<Helena::Events::EngineShutdown>(&TestSystem::OnEvent);
    }
    ~TestSystem() = default;

    void OnEvent(const Helena::Events::EngineInit&) {
        HELENA_MSG_DEBUG("EventInit");

        auto& ecs = Helena::Engine::GetSystem<Helena::Systems::EntityComponent>();

        const auto entity = ecs.CreateEntity();
        ecs.AddComponent<UserInfo>(entity, "Helena", 30u);

        const auto& userInfo = ecs.GetComponent<UserInfo>(entity);
        HELENA_MSG_DEBUG("User name: {}, age: {}", userInfo.name, userInfo.age);

        ecs.RemoveEntity(entity);
    }

    void OnEvent(const Helena::Events::EngineConfig&) {
        HELENA_MSG_DEBUG("EngineConfig");
    }

    void OnEvent(const Helena::Events::EngineExecute&) {
        HELENA_MSG_DEBUG("EngineExecute");
    }

    void OnEvent(const Helena::Events::EngineTick&) {
        HELENA_MSG_DEBUG("EngineTick");
    }

    void OnEvent(const Helena::Events::EngineUpdate&) {
        HELENA_MSG_DEBUG("EngineUpdate");
    }

    void OnEvent(const Helena::Events::EngineFinalize&) {
        HELENA_MSG_DEBUG("EngineFinalize");
    }

    void OnEvent(const Helena::Events::EngineShutdown&) {
        HELENA_MSG_DEBUG("EngineShutdown");
    }
};

int main(int argc, char** argv)
{
    // Engine started from Initialize method
    Helena::Core::Context::Initialize<Helena::Core::Context>();     // Initialize Context (Context used in Engine)
    Helena::Core::Context::SetAppName("Test Framework");            // Set application name
    Helena::Core::Context::SetTickrate(60.f);                       // Set Update tickrate
    Helena::Core::Context::SetCallback([]()                         // Register systems happen in this callback
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