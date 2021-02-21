#include <Common/Helena.hpp>

struct Test {
    int val;
};

using namespace Helena;

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
        // Push args in Core
        Core::SetArgs(argc, argv);
        
        // Get args size
        HF_MSG_INFO("Args: {}", Core::GetArgs().size());
        
        // View arguments from vector of std::string_view
        for(const auto& arg : Core::GetArgs()) {
            HF_MSG_INFO("Arg: {}", arg);
        }
        
        // Create context and get pointer on created context
        if(auto ptr = Core::CreateCtx<Test>(10); ptr) 
        {
            HF_MSG_INFO("WTF Value: {}", ptr->val);

            // Get context pointer
            if(ptr = Core::GetCtx<Test>(); ptr) {
                HF_MSG_INFO("WTF Value: {}", ptr->val);
            }

            // Remove context from Core
            Core::RemoveCtx<Test>();
        }
    }
    // todo: PluginManager, EventManager, NetManager, LogManager, JobManager, EntityManager
    return 0;
}