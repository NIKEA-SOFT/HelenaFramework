#include <Helena/Helena.hpp>

int main(int argc, char** argv)
{
    const auto wtf = Helena::Types::DateTime::FromString("%D/%M/%Y %h:%m:%s:%ms", "12/12/2024 24:12:7055");
    if(wtf.IsNull()) {
        HELENA_MSG_ERROR("Parse datetime failed");
    }

    //Engine started from Initialize method
    Helena::Engine::Context::Initialize();                  // Initialize Context (Context used in Engine)
    Helena::Engine::Context::SetAppName("Helena");          // Set application name
    Helena::Engine::Context::SetTickrate(60.f);             // Set Update tickrate
    Helena::Engine::Context::SetMain([]() {                 // Register systems happen in this callback
        // Register systems or initialize your context

        //Helena::Engine::RegisterSystem<T>(args...);
        //Helena::Engine::GetSystem<T>();
    });
    
    // Engine loop
    while(Helena::Engine::Heartbeat()) {}

    return 0;
}