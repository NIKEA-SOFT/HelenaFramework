#include <Helena/Helena.hpp>

class TestSystem
{
public:
    TestSystem() {
        // Start listen Engine events
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>(&TestSystem::OnInit);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Config>(&TestSystem::OnConfig);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Execute>(&TestSystem::OnExecute);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Tick>(&TestSystem::OnTick);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Update>(&TestSystem::OnUpdate);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Render>(&TestSystem::OnRender);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Finalize>(&TestSystem::OnFinalize);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Shutdown>(&TestSystem::OnShutdown);
    }
    ~TestSystem() = default;

    // drop event argument if event type -> std::is_empty_v
    void OnInit() {
        HELENA_MSG_DEBUG("EventInit");
    }

    void OnConfig() {
        HELENA_MSG_DEBUG("EngineConfig");
    }

    void OnExecute() {
        HELENA_MSG_DEBUG("EngineExecute");
    }

    void OnTick(const Helena::Events::Engine::Tick event) {
        HELENA_MSG_DEBUG("EngineTick: {:.4f}", event.deltaTime);
    }

    void OnUpdate(const Helena::Events::Engine::Update event) {
        HELENA_MSG_DEBUG("EngineUpdate: {:.4f}", event.fixedTime);
    }

    void OnRender(const Helena::Events::Engine::Render event) {
        HELENA_MSG_DEBUG("EngineRender: {:.4f}", event.deltaTime);
    }

    void OnFinalize() {
        HELENA_MSG_DEBUG("EngineFinalize");
    }

    void OnShutdown() {
        HELENA_MSG_DEBUG("EngineShutdown");
    }
};

int main(int argc, char** argv)
{
    //Engine started from Initialize method
    Helena::Engine::Context::Initialize();                  // Initialize Context (Context used in Engine)
    Helena::Engine::Context::SetAppName("Helena");          // Set application name
    Helena::Engine::Context::SetTickrate(30.f);             // Set Update tickrate
    Helena::Engine::Context::SetMain([]() {                 // Register systems happen in this callback
        Helena::Engine::RegisterSystem<TestSystem>();
    });

    // Engine loop
    while(Helena::Engine::Heartbeat()) {}

    return 0;
}