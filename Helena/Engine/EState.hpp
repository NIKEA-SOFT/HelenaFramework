#ifndef HELENA_ENGINE_STATE_HPP
#define HELENA_ENGINE_STATE_HPP

#include <cstdint>

namespace Helena::Core
{
    enum class EState : std::uint8_t
    {
        Undefined,
        Init,
        Shutdown
    };
}

#endif // HELENA_ENGINE_STATE_HPP
