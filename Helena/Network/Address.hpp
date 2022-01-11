#ifndef HELENA_NETWORK_ADDRESS_HPP
#define HELENA_NETWORK_ADDRESS_HPP

#include <Helena/Types/FixedBuffer.hpp>

namespace Helena::Network
{
    struct Address 
    {
        using Buffer = Types::FixedBuffer<46>;

        constexpr Address(Buffer ip, std::uint16_t port) : m_IP{ip}, m_Port{port} {};
        constexpr Address() : m_IP{}, m_Port{} {}
        ~Address() = default;
        constexpr Address(const Address&) = default;
        constexpr Address(Address&&) noexcept = default;
        constexpr Address& operator=(const Address&) = default;
        constexpr Address& operator=(Address&&) noexcept = default;

        void SetIP(const char* ip) noexcept {
            m_IP = ip;
        }

        [[nodiscard]] constexpr const Buffer& GetIP() const noexcept {
            return m_IP;
        }

        void SetPort(std::uint16_t port) noexcept {
            m_Port = port;
        }

        [[nodiscard]] constexpr std::uint16_t GetPort() const noexcept {
            return m_Port;
        }

        [[nodiscard]] constexpr bool operator==(const Address& other) noexcept {
            return m_IP == other.m_IP && m_Port == other.m_Port;
        }

        [[nodiscard]] constexpr bool operator!=(const Address& other) noexcept {
            return !(m_IP == other.m_IP && m_Port == other.m_Port);
        }

        [[nodiscard]] constexpr bool Empty() const noexcept {
            return m_IP.Empty();
        }

    private:
        Types::FixedBuffer<46> m_IP;
        std::uint16_t m_Port;
    };
}

#endif // HELENA_NETWORK_ADDRESS_HPP
