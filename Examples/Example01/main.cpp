#include <Helena/Helena.hpp>

#if defined(HELENA_PLATFORM_LINUX)
int main(int argc, char** argv) {
    HELENA_MSG_ERROR("Sorry, this example only for Windows platform :(");
    return 0;
}
#endif

#if defined(HELENA_PLATFORM_WIN)
#include "Application.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Engine started from Initialize method
    // Here used own Context
    Helena::Engine::Context::Initialize<Example01::Application>(/*args for ctor...*/);

    // Possible other syntax when used own context
    // Example01::Application::Initialize<Example01::Application>(/*args for ctor...*/);

    // Engine loop
    while(Helena::Engine::Heartbeat()) {}

    return 0;
}
#endif