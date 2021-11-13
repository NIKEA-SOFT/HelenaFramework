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
        IEventPool() = default;
        virtual ~IEventPool() = default;
        IEventPool(const IEventPool&) = default;
        IEventPool(IEventPool&&) noexcept = default;
        IEventPool& operator=(const IEventPool&) = default;
        IEventPool& operator=(IEventPool&&) noexcept = default;
    };
}

#endif // HELENA_CORE_EVENTS_HPP
