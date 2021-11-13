#ifndef HELENA_CORE_EVENTS_HPP
#define HELENA_CORE_EVENTS_HPP

namespace Helena::Events
{
    struct EngineInit {};
    struct EngineConfig {};
    struct EngineExecute {};
    struct EngineTick {};
    struct EngineUpdate {};
    struct EngineFinalize {};
    struct EngineShutdown {};
}

namespace Helena::Core
{
    struct IEventPool {
        virtual ~IEventPool() = default;
    };
}

#endif // HELENA_CORE_EVENTS_HPP
