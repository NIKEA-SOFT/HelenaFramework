#include <Common/Helena.hpp>

struct Test {
    int val;
};

int main(int argc, char** argv)
{
    if(Helena::Core::Initialize()) 
    {
        // Push args in Core
        Helena::Core::SetArgs(argc, argv);
        
        // Get args size
        HF_MSG_INFO("Args: {}", Helena::Core::GetArgs().size());
        
        // View arguments from vector of std::string_view
        for(const auto arg : Helena::Core::GetArgs()) {
            HF_MSG_INFO("Arg: {}", arg);
        }

        // Create context and get pointer on created context
        if(auto ptr = Helena::Core::CreateCtx<Test>(10); ptr) 
        {
            HF_MSG_INFO("WTF Value: {}", ptr->val);

            // Get context pointer
            if(ptr = Helena::Core::GetCtx<Test>(); ptr) {
                HF_MSG_INFO("WTF Value: {}", ptr->val);
            }

            // Remove context from Core
            Helena::Core::RemoveCtx<Test>();
        }
    }
    // todo: PluginManager, EventManager, NetManager, LogManager, JobManager, EntityManager
    return 0;
}