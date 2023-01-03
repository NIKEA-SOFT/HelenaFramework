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
        double deltaTime;
    };
    struct Tick : PreTick {};
    struct PostTick : PreTick {};


    struct PreUpdate {
        double fixedTime;
    };
    struct Update : PreUpdate {};
    struct PostUpdate : PreUpdate {};


    struct PreRender {
        double alpha;
        double deltaTime;
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
