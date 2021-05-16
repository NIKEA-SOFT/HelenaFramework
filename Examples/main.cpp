#include <Helena/Helena.hpp>

// Systems
#include <Helena/Systems/EntityComponent.hpp>
#include <Helena/Systems/ConfigManager.hpp>

using namespace Helena;
using namespace Helena::Hash::Literals;

namespace Helena::Components 
{
    struct Velocity {
        float m_Speed;
    };

    struct Position {
        float x;
        float y;
        float z;
    };

    // Tag components
    struct PlayerState {
        using Die = Systems::EntityComponent::Tag<"PlayerState_Die"_hs>;
    };
}

struct TestSystem 
{

    using Entity = Systems::EntityComponent::Entity;

    template <Systems::ConfigManager::ID Value>
    using Resource = Systems::ConfigManager::Resource<Value>;

    template <Systems::ConfigManager::ID Value>
    using Key = Systems::ConfigManager::Key<Value>;

    // Current method called when system initialized (it's system event)
    void OnSystemCreate() {
        HF_MSG_DEBUG("OnSystemCreate");

        // Test Core events
        Core::RegisterEvent<Events::Initialize,     &TestSystem::OnCoreInitialize>(this);
        Core::RegisterEvent<Events::Finalize,       &TestSystem::OnCoreFinalize>(this);

        // Test Events for EntityComponent
        Core::RegisterEvent<Events::Systems::EntityComponent::CreateEntity, &TestSystem::OnCreateEntity>(this);
        Core::RegisterEvent<Events::Systems::EntityComponent::RemoveEntity, &TestSystem::OnRemoveEntity>(this);

        Core::RegisterEvent<Events::Systems::EntityComponent::AddComponent<Components::Position>,       &TestSystem::OnComponentAddPosition>(this);
        Core::RegisterEvent<Events::Systems::EntityComponent::RemoveComponent<Components::Position>,    &TestSystem::OnComponentRemovePosition>(this);

        Core::RegisterEvent<Events::Systems::EntityComponent::AddComponent<Components::Velocity>,       &TestSystem::OnComponentAddVelocity>(this);
        Core::RegisterEvent<Events::Systems::EntityComponent::RemoveComponent<Components::Velocity>,    &TestSystem::OnComponentRemoveVelocity>(this);

        Core::RegisterEvent<Events::Systems::EntityComponent::AddComponent<Components::PlayerState::Die>,       &TestSystem::OnComponentAddPlayerStateDie>(this);
        Core::RegisterEvent<Events::Systems::EntityComponent::RemoveComponent<Components::PlayerState::Die>,    &TestSystem::OnComponentRemovePlayerStateDie>(this);
    }

    enum class Item {};
    // Current method called after OnSystemCreate (it's system event)
    void OnSystemExecute() {
        HF_MSG_DEBUG("OnSystemExecute");

        //test ecs system
        auto& ecs = Core::GetSystem<Systems::EntityComponent>();
        std::vector<Entity> entities; entities.resize(5);       // Create vector with elements
        ecs.CreateEntity(entities.begin(), entities.end());    // Fill vector with entities

        // iterate each entity
        for(const auto id : entities) {
            ecs.AddComponent<Components::Position>(id, 1.f, 1.f, 1.f); // Set component to entity
            ecs.AddComponent<Components::PlayerState::Die>(id); // Set tag component to entity
        }

        // View/Group
        const auto view = ecs.ViewComponent<Components::PlayerState::Die>();   // Search all entities with component
        for([[maybe_unused]] const auto id : view) {
            // Player: <id> is die...
        }

        const auto group = ecs.GroupComponent<Components::Position>(Systems::EntityComponent::Get<Components::PlayerState::Die>);
        for(const auto id : group) {
            auto& pos = group.get<Components::Position>(id);
                
            pos.x = 5.f;
            pos.y = 5.f;
            pos.z = 5.f;

            // Let's remove components from each entity
            ecs.RemoveComponent<Components::Position, Components::PlayerState::Die>(id);
        }

        // Destroy all entities and remove components
        ecs.Clear();

        // Test ConfigManager
        auto& configManager = Core::GetSystem<Systems::ConfigManager>();
        configManager.CreateResource<Components::Velocity>(1.f);
        configManager.CreateResource<Components::Position>(1.f, 2.f, 3.f);
        if(configManager.HasResource<Components::Velocity, Components::Position>()) {
            const auto& [vel, pos] = configManager.GetResource<Components::Velocity, Components::Position>();
            configManager.RemoveResource<Components::Velocity, Components::Position>();
        }

        if(configManager.AnyResource<Components::Velocity, Components::Position, Components::PlayerState>()) {
            HF_MSG_WARN("AnyResource success");
        }

        using gameResourceKey = Resource<"Game"_hs>;
        using gamePropertyCapKey = Key<"Cap"_hs>;
        configManager.AddProperty<gameResourceKey, gamePropertyCapKey, std::uint32_t>(100);

    }

    void OnComponentAddPosition(const Events::Systems::EntityComponent::AddComponent<Components::Position>& event) {
        using Position = Internal::remove_cvref_t<decltype(event)>;
        HF_MSG_DEBUG("Entity: {} component Position add!", event.m_Entity);
    }

    void OnComponentRemovePosition(const Events::Systems::EntityComponent::RemoveComponent<Components::Position>& event) {
        using Position = Internal::remove_cvref_t<decltype(event)>;
        HF_MSG_DEBUG("Entity: {} component Position removed!", event.m_Entity);
    }

    void OnComponentAddVelocity(const Events::Systems::EntityComponent::AddComponent<Components::Velocity>& event) {
        using Velocity = Internal::remove_cvref_t<decltype(event)>;
        HF_MSG_DEBUG("Entity: {} component Velocity add!", event.m_Entity);
    }

    void OnComponentRemoveVelocity(const Events::Systems::EntityComponent::RemoveComponent<Components::Velocity>& event) {
        using Velocity = Internal::remove_cvref_t<decltype(event)>;
        HF_MSG_DEBUG("Entity: {} component Velocity removed!", event.m_Entity);
    }

    void OnComponentAddPlayerStateDie(const Events::Systems::EntityComponent::AddComponent<Components::PlayerState::Die>& event) {
        using PlayerStateDie = Internal::remove_cvref_t<decltype(event)>;
        HF_MSG_DEBUG("Entity: {} component PlayerState::Die add!", event.m_Entity);
    }

    void OnComponentRemovePlayerStateDie(const Events::Systems::EntityComponent::RemoveComponent<Components::PlayerState::Die>& event) {
        using Velocity = Internal::remove_cvref_t<decltype(event)>;
        HF_MSG_DEBUG("Entity: {} component PlayerState::Die removed!", event.m_Entity);
    }

    void OnCreateEntity(const Events::Systems::EntityComponent::CreateEntity& event) {
        HF_MSG_DEBUG("OnCreateEntity: {} event called", event.m_Entity);
    }

    void OnRemoveEntity(const Events::Systems::EntityComponent::RemoveEntity& event) {
        HF_MSG_DEBUG("OnRemoveEntity: {} event called", event.m_Entity);
    }

    // Called when Core initialized
    void OnCoreInitialize(const Events::Initialize& event) {
        HF_MSG_DEBUG("OnCoreInitialize");
    }
    // Called when Core finish
    void OnCoreFinalize(const Events::Finalize& event) {
        HF_MSG_DEBUG("OnCoreFinalize");
    }

    // Called every tick (it's system event)
    void OnSystemTick() {
        //HF_MSG_DEBUG("OnSystemTick");
    }

    // Called every tickrate (fps) tick (default: 0.016 ms) (it's system event)
    void OnSystemUpdate() {
        //HF_MSG_DEBUG("OnSystemUpdate, delta: {:.4f}", Core::GetTimeDelta());
    }

    // Called when System destroyed (it's system event)
    void OnSystemDestroy() {
        HF_MSG_DEBUG("OnSystemDestroy");

        Core::RemoveEvent<Events::Initialize,   &TestSystem::OnCoreInitialize>(this);
        Core::RemoveEvent<Events::Finalize,     &TestSystem::OnCoreFinalize>(this);

        Core::RemoveEvent<Events::Systems::EntityComponent::CreateEntity, &TestSystem::OnCreateEntity>(this);
        Core::RemoveEvent<Events::Systems::EntityComponent::RemoveEntity, &TestSystem::OnRemoveEntity>(this);

        Core::RemoveEvent<Events::Systems::EntityComponent::AddComponent<Components::Position>,       &TestSystem::OnComponentAddPosition>(this);
        Core::RemoveEvent<Events::Systems::EntityComponent::RemoveComponent<Components::Position>,    &TestSystem::OnComponentRemovePosition>(this);

        Core::RemoveEvent<Events::Systems::EntityComponent::AddComponent<Components::Velocity>,       &TestSystem::OnComponentAddVelocity>(this);
        Core::RemoveEvent<Events::Systems::EntityComponent::RemoveComponent<Components::Velocity>,    &TestSystem::OnComponentRemoveVelocity>(this);
    }
};


int main(int argc, char** argv)
{
    // Helena example
    return Core::Initialize([&]() -> bool {
        // Push args in Core
        Core::SetArgs(argc, argv);

        //Core::SetArgs(argc, argv);
        // Set tickrate (30 fps)
        Core::SetTickrate(60.0);

        // Get args size
        HF_MSG_INFO("Args: {}", Core::GetArgs().size());
        
        // View arguments from vector of std::string_view
        for(const auto& arg : Core::GetArgs()) {
            HF_MSG_INFO("Arg: {}", arg);
        }
        
        // test create
        Core::RegisterSystem<Systems::ConfigManager>();
        Core::RegisterSystem<Systems::EntityComponent>();
        Core::RegisterSystem<TestSystem>();

        return true;
    });
}
