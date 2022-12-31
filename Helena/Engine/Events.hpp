#ifndef HELENA_ENGINE_EVENTS_HPP
#define HELENA_ENGINE_EVENTS_HPP

namespace Helena::Events::Engine
{
    struct PreInit {};
    struct Init {};
    struct PostInit {};


    struct PreConfig {};
    struct Config {};
    struct PostConfig {};


    struct PreExecute {};
    struct Execute {};
    struct PostExecute {};


    struct PreTick {
        float deltaTime;
    };
    struct Tick : PreTick {};
    struct PostTick : PreTick {};


    struct PreUpdate {
        float fixedTime;
    };
    struct Update : PreUpdate {};
    struct PostUpdate : PreUpdate {};


    struct PreRender {
        float alpha;
        float deltaTime;
    };
    struct Render : PreRender {};
    struct PostRender : PreRender {};


    struct PreFinalize {};
    struct Finalize {};
    struct PostFinalize {};


    struct PreShutdown {};
    struct Shutdown {};
    struct PostShutdown {};
}

#endif // HELENA_ENGINE_EVENTS_HPP
