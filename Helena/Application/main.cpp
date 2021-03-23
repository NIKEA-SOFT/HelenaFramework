#include <Common/Helena.hpp>

using namespace Helena;

struct TestSystem {

    TestSystem(std::string_view text) : text{text} {}
    ~TestSystem() = default;

    void Create() {
        HF_MSG_INFO("System event Create");
        
        Core::RegisterEvent<Events::Core::HeartbeatBegin, &TestSystem::HeartbeatBegin>(this);
        Core::RegisterEvent<Events::Core::HeartbeatEnd, &TestSystem::HeartbeatEnd>(this);
        Core::RegisterEvent<Events::Core::TickPre, &TestSystem::TickPre>(this);
        Core::RegisterEvent<Events::Core::Tick, &TestSystem::Tick>(this);
        Core::RegisterEvent<Events::Core::TickPost, &TestSystem::TickPost>(this);
    }

    void HeartbeatBegin(const Events::Core::HeartbeatBegin& event) {
        HF_MSG_INFO("Core HeartbeatBegin event");
    }

    // Called every tick
    void Update() {
        HF_MSG_INFO("System event Update, elapsed: {:.4f}, delta: {:.4f}", Core::GetTimeElapsed(), Core::GetTimeDelta());
    }

    void TickPre(const Events::Core::TickPre& event) {
        HF_MSG_INFO("TickPre called");
    }

    void Tick(const Events::Core::Tick& event) {
        HF_MSG_INFO("Tick called, var = {}", text);
    }

    void TickPost(const Events::Core::TickPost& event) {
        HF_MSG_INFO("TickPost called");
    }

    void HeartbeatEnd(const Events::Core::HeartbeatEnd& event) {
        HF_MSG_INFO("Core HeartbeatEnd event");
    }

    void Destroy() {
        HF_MSG_INFO("System event Destroy");

        Core::RemoveEvent<Events::Core::HeartbeatBegin, &TestSystem::HeartbeatBegin>(this);
        Core::RemoveEvent<Events::Core::HeartbeatEnd, &TestSystem::HeartbeatEnd>(this);
        Core::RemoveEvent<Events::Core::TickPre, &TestSystem::TickPre>(this);
        Core::RemoveEvent<Events::Core::Tick, &TestSystem::Tick>(this);
        Core::RemoveEvent<Events::Core::TickPost, &TestSystem::TickPost>(this);
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
        Core::SetArgs(argc, argv);
        // Set tickrate (30 fps)
        Core::SetTickrate(30);
        

        // Get args size
        HF_MSG_INFO("Args: {}", Core::GetArgs().size());
        
        // View arguments from vector of std::string_view
        for(const auto& arg : Core::GetArgs()) {
            HF_MSG_INFO("Arg: {}", arg);
        }

        // test create
        if(auto ptr = Core::RegisterSystem<TestSystem>("WTF"); ptr) {
            HF_MSG_INFO("Create system: {} success", entt::type_name<TestSystem>().value());
        } else {
            HF_MSG_ERROR("Create system: {} failure", entt::type_name<TestSystem>().value());
        }

        return true;
    });
}