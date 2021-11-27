#include <Helena/Engine/Engine.hpp>

#include <Helena/Systems/EntityComponent.hpp>
#include <Helena/Systems/ResourceManager.hpp>

#include <Helena/Types/FixedBuffer.hpp>
#include <Helena/Types/VectorKVAny.hpp>

#include <Helena/Dependencies/EnTT.hpp>

#include <Helena/Platform/Platform.hpp>

#include <Helena/Util/Cast.hpp>

using namespace Helena;

// Components for Entity
namespace Components 
{
    struct Info {
        Types::FixedBuffer<32> name;
        std::uint64_t connection;
    };

    struct Health {
        std::uint32_t health;
    };

    struct Position {
        float x;
        float y;
        float z;
    };

    struct Velocity 
    {
        float x;
        float y;
        float z;
    };

    struct ModelInfo {
        std::uint32_t id;
        std::uint32_t size;
        std::uint32_t bones;
    };
}

// system for create/remove character entity
class CharacterSystem 
{
    using EntityID = Systems::EntityComponent::Entity;

public:
    void CreateCharacter(std::uint64_t connection, Types::FixedBuffer<32> name) 
    {
        // Get ref on ECS system
        auto& ecs = Engine::GetSystem<Systems::EntityComponent>();

        // Create new entity
        const EntityID id = ecs.CreateEntity();

        // Add new components to entity
        ecs.AddComponent<Components::Info>(id, name, connection);
        ecs.AddComponent<Components::ModelInfo>(id, 10u, 150u, 5u);
        ecs.AddComponent<Components::Position>(id, 30.f, 50.f, 10.f);
        ecs.AddComponent<Components::Velocity>(id, 0.f, 0.f, 0.f);
        ecs.AddComponent<Components::Health>(id, 100u);

        m_Characters.emplace(connection, id);

        HELENA_MSG_INFO("Create entity: {} for connection: {}", id, connection);
        PrintCharacterInfo(connection);
    }

    void RemoveCharacter(std::uint64_t connection) 
    {
        if(const auto entity = FindCharacter(connection)) {
            auto& ecs = Engine::GetSystem<Systems::EntityComponent>();
            ecs.RemoveEntity(entity.value());
            m_Characters.erase(connection);
            HELENA_MSG_INFO("Connection: {} entity: {} removed!", connection, entity.value());
        }
    }

    void PrintCharacterInfo(std::uint64_t connection) 
    {
        if(const auto entity = FindCharacter(connection)) 
        {
            auto& ecs = Engine::GetSystem<Systems::EntityComponent>();
            
            // Get entity components
            auto [info, health, position, velocity, model] = ecs.GetComponent<Components::Info, Components::Health, 
                Components::Position, Components::Velocity, Components::ModelInfo>(entity.value());

            HELENA_MSG_INFO(
                "\n--------- [Connection: {}] --------\n"
                "ID: {}\n"
                "Name: {}\n"
                "Health: {}\n"
                "Position X: {}, Y: {}, Z: {}\n"
                "Velocity X: {}, Y: {}, Z: {}\n"
                "Model info ID: {}, Size: {}, Bones: {}\n"
                "-----------------------------------\n", 
                connection, 
                entity.value(), 
                info.name, 
                health.health, 
                position.x, position.y, position.z,
                velocity.x, velocity.y, velocity.z,
                model.id, model.size, model.bones);

        } else HELENA_MSG_ERROR("Connection: {} entity: {} not found!", connection, entity.value());

    }

private:

    // Find Entity from connection id
    std::optional<EntityID> FindCharacter(std::uint64_t connection) {
        const auto it = m_Characters.find(connection);
        return it != m_Characters.cend() ? std::make_optional<EntityID>(it->second) : std::nullopt;
    }

    // Pair [connection <-> entity]
    std::unordered_map<std::uint64_t, Systems::EntityComponent::Entity> m_Characters;
};


// Test PhysicsSystem 
// Events for PhysicSystem
namespace Helena::Events::PhysicSystem
{
    // Event structure
    struct EntityMove {
        using Entity = Systems::EntityComponent::Entity;
        const Entity m_Entity;
        const Components::Position& m_Position;
        const Components::Velocity& m_Velocity;
    };
}

class PhysicsSystem
{
public:
    PhysicsSystem() {
        // Start listen Engine event Update (called with fixed time)
        Engine::SubscribeEvent<Events::Engine::Update>(&PhysicsSystem::EventUpdate);
    }

    void EventUpdate(const Events::Engine::Update&)
    {
        // Get ref on used systems
        auto [ecs, charSystem] = Engine::GetSystem<Systems::EntityComponent, CharacterSystem>();

        // Get pool on entities with current components
        const auto view = ecs.ViewComponent<Components::Info, Components::Position, Components::Velocity>();
        for(auto id : view) 
        {
            // Get ref on components
            auto [info, position, velocity] = view.get(id);

            position.x += 0.1f;
            position.y += 0.1f;
            position.z += 0.1f;

            velocity.x += 10.f;
            velocity.y += 10.f;
            velocity.z += 10.f;

            Engine::SignalEvent<Events::PhysicSystem::EntityMove>(id, std::cref(position), std::cref(velocity));

            // Remove character entity if velocity >= 100
            if(velocity.x >= 100.f) {
                charSystem.RemoveCharacter(info.connection);
                Engine::Shutdown("test finished!");
            }
        }
    }
};


// Let's emulate network packet "CS_CreateCharacter"
void NetworkCreateCharacter() 
{
    constexpr std::uint64_t connection_ivan = 0;      // Test emulation network connection
    constexpr std::uint64_t connection_petr = 1;      // Test emulation network connection
    constexpr std::uint64_t connection_alex = 2;      // Test emulation network connection

    auto& characterSystem = Engine::GetSystem<CharacterSystem>(); // Get ref on system

    // Create character entity for test
    characterSystem.CreateCharacter(connection_ivan, "Ivan");
    characterSystem.CreateCharacter(connection_petr, "Petr");
    characterSystem.CreateCharacter(connection_alex, "Alex");

    // Listener on event EntityMove
    Engine::SubscribeEvent<Events::PhysicSystem::EntityMove>([](const Events::PhysicSystem::EntityMove& event) {
        auto& [entity, pos, vel] = event;
        HELENA_MSG_INFO("[Event: EntityMove] Entity: {} move to X: {:.3f}, Y: {:.3f}, Z: {:.3f}", entity, pos.x, pos.y, pos.z);
    });
}


int main(int argc, char** argv)
{
    // Engine started from Initialize method
    Engine::Context::Initialize();     // Initialize Context (Context used in Engine)
    Engine::Context::SetAppName("Test Framework");    // Set application name
    Engine::Context::SetTickrate(1.f);                // Set Update tickrate
    Engine::Context::SetCallback([]() -> void         // Register systems happen in this callback
    {
        // Register all used systems
        //Engine::RegisterSystem<Systems::NetworkManager>();      // Network system
        Engine::RegisterSystem<Systems::EntityComponent>();     // Entity Component System
        Engine::RegisterSystem<CharacterSystem>();              // Test System for Characters
        Engine::RegisterSystem<PhysicsSystem>();                // Test Physic system for show example

        NetworkCreateCharacter();                               // Only for emulate new network connection
    });
    
    // Engine loop
    while(Engine::Heartbeat()) {}

    return 0;
}