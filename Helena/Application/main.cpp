#include <Common/Helena.hpp>

using namespace Helena;

struct TestSystem {

    TestSystem(std::string_view text) : text{text} {}
    ~TestSystem() = default;

    void OnCreate() {
        HF_MSG_INFO("OnCreate called zxc");
        Core::RegisterEvent<Events::Core::UpdatePre, &TestSystem::OnUpdatePre>(this);
        Core::RegisterEvent<Events::Core::Update, &TestSystem::OnUpdate>(this);
        Core::RegisterEvent<Events::Core::UpdatePost, &TestSystem::OnUpdatePost>(this);
    }

    void OnUpdatePre(Events::Core::UpdatePre update) {
        HF_MSG_INFO("OnUpdatePre called");
        Core::RemoveEvent<Events::Core::UpdatePre, &TestSystem::OnUpdatePre>(this);
    }

    void OnUpdate(Events::Core::Update update) {
        HF_MSG_INFO("OnUpdate called");
        Core::RemoveEvent<Events::Core::Update, &TestSystem::OnUpdate>(this);
    }

    void OnUpdatePost(Events::Core::UpdatePost update) {
        HF_MSG_INFO("OnUpdatePost called");
        Core::RemoveEvent<Events::Core::UpdatePost, &TestSystem::OnUpdatePost>(this);
    }

    void OnDestroy() {
        HF_MSG_INFO("OnDestroy called");
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
    if(Core::Initialize()) 
    {
        const auto ctx = Core::GetContext();

        // Push args in Core
        Core::SetArgs(argc, argv);
        
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


        Core::Heartbeat(30.0f);

        //// test already exist
        //if(auto ptr = Core::RegisterSystem<Test>(); ptr) {
        //    HF_MSG_INFO("Create system: {} success, value: {}", entt::type_name<Test>().value(), ptr->value);
        //} else {
        //    HF_MSG_ERROR("Create system: {} failure", entt::type_name<Test>().value());
        //}

        //// test get and remove
        //if(auto ptr = Core::GetSystem<Test>(); ptr) {
        //    HF_MSG_INFO("System: {} exist", entt::type_name<Test>().value());
        //    Core::RemoveSystem<Test>();
        //}
    }


    // todo: PluginManager, EventManager, NetManager, LogManager, JobManager, EntityManager
    return 0;
}