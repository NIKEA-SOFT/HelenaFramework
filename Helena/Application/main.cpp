#include <Common/Helena.hpp>

using namespace Helena;

struct TestSystem {

    TestSystem(std::string_view text) : text{text} {}
    ~TestSystem() = default;

    void Create() {
        HF_MSG_INFO("System event Create");
        
        Core::RegisterEvent<Events::Core::TickPre, &TestSystem::TickPre>(this);
        Core::RegisterEvent<Events::Core::Tick, &TestSystem::Tick>(this);
        Core::RegisterEvent<Events::Core::TickPost, &TestSystem::TickPost>(this);

        //Core::RemoveSystem<TestSystem>();
    }

    void Update() {
        HF_MSG_INFO("System event Update, elapsed: {:.4f}, delta: {:.4f}", Core::GetTimeElapsed(), Core::GetTimeDelta());
    }

    void Destroy() {
        HF_MSG_INFO("System event Destroy");

        Core::RemoveEvent<Events::Core::TickPre, &TestSystem::TickPre>(this);
        Core::RemoveEvent<Events::Core::Tick, &TestSystem::Tick>(this);
        Core::RemoveEvent<Events::Core::TickPost, &TestSystem::TickPost>(this);
    }

    void TickPre(Events::Core::TickPre update) {
        HF_MSG_INFO("TickPre called");
        Core::RemoveEvent<decltype(update), &TestSystem::TickPre>(this);
    }

    void Tick(Events::Core::Tick update) {
        HF_MSG_WARN("Tick called, var = {}", text);
    }

    void TickPost(Events::Core::TickPost update) {
        HF_MSG_INFO("TickPost called");
        Core::RemoveEvent<decltype(update), &TestSystem::TickPost>(this);
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