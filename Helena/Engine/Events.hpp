#ifndef HELENA_ENGINE_EVENTS_HPP
#define HELENA_ENGINE_EVENTS_HPP

namespace Helena::Events::Engine
{
    struct Init {};
    struct Config {};
    struct Execute {};

    struct Tick {
        float deltaTime;
    };

    struct Update {
        float fixedTime;
    };

    struct Render {
        float alpha;
        float deltaTime;
    };

    struct Finalize {};
    struct Shutdown {};
}

#endif // HELENA_ENGINE_EVENTS_HPP
