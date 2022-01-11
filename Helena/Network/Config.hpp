#ifndef HELENA_NETWORK_CONFIG_HPP
#define HELENA_NETWORK_CONFIG_HPP

#include <cstdint>
#include <cstddef>

namespace Helena::Network
{
    struct Config 
    {
        Config(std::uint16_t peers, std::uint8_t channels, std::uint32_t bandwidthIn = 0, std::uint32_t bandwidthOut = 0) 
            : m_MaxPeers{peers}
            , m_MaxChannels{channels}
            , m_BandwidthIn{bandwidthIn}
            , m_BandwidthOut{bandwidthOut} {}
        ~Config() = default;
        Config(const Config&) = default;
        Config(Config&&) noexcept = default;
        Config& operator=(const Config&) = default;
        Config& operator=(Config&&) noexcept = default;

        std::uint16_t   m_MaxPeers;
        std::uint8_t    m_MaxChannels;
        std::uint32_t   m_BandwidthIn;
        std::uint32_t   m_BandwidthOut;
    };
}

#endif // HELENA_NETWORK_CONFIG_HPP
