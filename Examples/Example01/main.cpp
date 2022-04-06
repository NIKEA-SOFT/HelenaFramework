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
    //Engine started from Initialize method
    Example01::Application::Initialize<Example01::Application>();
    Example01::Application::SetAppName("Example01");
    Example01::Application::SetWindowSize(960, 840);
    Example01::Application::SetTickrate(60.f);
    Example01::Application::SetMain([]() {
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>(&Example01::Application::OnInitialize);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Tick>(&Example01::Application::OnTick);
    });
    
    // Engine loop
    while(Helena::Engine::Heartbeat()) {}

    return 0;
}
#endif