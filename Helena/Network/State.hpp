#ifndef HELENA_NETWORK_STATE_HPP
#define HELENA_NETWORK_STATE_HPP

#include <cstdint>
#include <cstddef>

namespace Helena::Network
{
    enum class EState : std::uint8_t {
        Disconnected,
        Connected
    };
}

#endif // HELENA_NETWORK_STATE_HPP
