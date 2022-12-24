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
    Example01::Application::Initialize<Example01::Application>();

    // Engine loop
    while(Helena::Engine::Heartbeat()) {}

    return 0;
}
#endif