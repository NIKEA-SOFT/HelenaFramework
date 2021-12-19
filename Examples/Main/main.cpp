#include <Helena/Helena.hpp>

int main(int argc, char** argv)
{
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