#include <Common/Helena.hpp>

using namespace Helena;

namespace Helena::Components 
{
    struct Velocity {
        std::uint32_t wtf;
    };

    struct Position {
        float x;
        float y;
        float z;
    };
}

struct TestSystem 
{
    // Called 
    void Create() {
        HF_MSG_INFO("System event Create");
        
        Core::RegisterEvent<Events::CoreInit,       &TestSystem::OnEventCoreInit>(this);
        Core::RegisterEvent<Events::CoreFinish,     &TestSystem::OnEventCoreFinish>(this);
        Core::RegisterEvent<Events::CoreTickPre,    &TestSystem::OnEventCoreTickPre>(this);
        Core::RegisterEvent<Events::CoreTick,       &TestSystem::OnEventCoreTick>(this);
        Core::RegisterEvent<Events::CoreTickPost,   &TestSystem::OnEventCoreTickPost>(this);
    }


    // Called every N milliseconds (depending on tickrate, default: 30 FPS)
    void Update() {}
    // Called when Core initialized
    void OnEventCoreInit(const Events::CoreInit& event) {}
    // Called when Core finish
    void OnEventCoreFinish(const Events::CoreFinish& event) {}
    // Called before Tick
    void OnEventCoreTickPre(const Events::CoreTickPre& event) {}
    // Called after TickPre
    void OnEventCoreTick(const Events::CoreTick& event) {}
    // Called after Tick
    void OnEventCoreTickPost(const Events::CoreTickPost& event) {}

    void Destroy() {
        Core::RemoveEvent<Events::CoreInit,     &TestSystem::OnEventCoreInit>(this);
        Core::RemoveEvent<Events::CoreFinish,   &TestSystem::OnEventCoreFinish>(this);
        Core::RemoveEvent<Events::CoreTickPre,  &TestSystem::OnEventCoreTickPre>(this);
        Core::RemoveEvent<Events::CoreTick,     &TestSystem::OnEventCoreTick>(this);
        Core::RemoveEvent<Events::CoreTickPost, &TestSystem::OnEventCoreTickPost>(this);
    }

    std::string text;
};

int main(int argc, char** argv)
{
    // Lua example
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package);
    lua.script("print('hello lua world!')");

    if(auto script = lua.load_file(std::filesystem::path{argv[0]}.parent_path().string() + HF_SEPARATOR + "test.lua"); script.valid()) 
    {
        if(const auto fnScript = script(); fnScript.valid()) {
            HF_MSG_INFO("Lua script exec success!");
        } else {
            HF_MSG_ERROR("Lua script exec failed!");
        }
    } else {
        HF_MSG_ERROR("Load script file failed");
    }


    // Helena example
    return Core::Initialize([&]() -> bool {
        // Push args in Core
        //Core::SetArgs(argc, argv);
        // Set tickrate (30 fps)
        Core::SetTickrate(30);
        

        // Get args size
        HF_MSG_INFO("Args: {}", Core::GetArgs().size());
        
        // View arguments from vector of std::string_view
        for(const auto& arg : Core::GetArgs()) {
            HF_MSG_INFO("Arg: {}", arg);
        }

        // test create
        Core::RegisterSystem<TestSystem>();
        Core::RegisterSystem<Systems::EntityComponent>();
        Core::RegisterSystem<Systems::ConfigManager>();

        return true;
    });
}