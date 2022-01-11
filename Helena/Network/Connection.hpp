#ifndef HELENA_NETWORK_CONNECTION_HPP
#define HELENA_NETWORK_CONNECTION_HPP

#include <Helena/Network/Address.hpp>
#include <Helena/Network/Message.hpp>
#include <Helena/Network/UserData.hpp>

namespace Helena::Network
{
    enum class EDisconnect : std::uint8_t
    {
        Default,    // Disconnect only after all queued outgoing packets are sent
        Update,     // Disconnect only after call Update (packets will not be sent)
        Now         // Disconnect forced (packets will not be sent)
    };

    class Network;
    class Connection 
    {
    public:
        Connection(Network* network, ENetPeer* peer, EState state) 
            : m_Network{network}, m_Peer{peer}, m_State{state} {}
        ~Connection() = default;
        Connection(const Connection&) = delete;
        Connection(Connection&&) noexcept = delete;
        Connection& operator=(const Connection&) = delete;
        Connection& operator=(Connection&&) noexcept = delete;

        [[nodiscard]] std::uint32_t GetID() const noexcept {
            return m_Peer->connectID;
        }

        [[nodiscard]] bool Send(const Message& message) const
        {
            if(m_State == EState::Disconnected) {
                return false;
            }
            
            HELENA_ASSERT(message.m_Packet, "Packet is nullptr");
            if(!message.m_Packet) {
                return false;
            }

            HELENA_ASSERT(message.m_Packet->dataLength, "Data is empty");
            if(!message.m_Packet->dataLength) {
                return false;
            }

            HELENA_ASSERT(message.m_RefCounter, "How it's happen, why m_RefCounter is null?");
            if(!message.m_RefCounter) {
                return false;
            }

            if(message.m_RefCounter == 1) {
                message.m_RefCounter++;
            }

            return !enet_peer_send(m_Peer, message.m_Channel, message.m_Packet);
        }

        void Disconnect(std::uint32_t data = 0, EDisconnect type = EDisconnect::Default)
        {
            switch(type)
            {
                case EDisconnect::Default:  enet_peer_disconnect_later(m_Peer, data); break;
                case EDisconnect::Update:   enet_peer_disconnect(m_Peer, data); break;
                case EDisconnect::Now:      enet_peer_disconnect_now(m_Peer, data); break;
            }
        }

        [[nodiscard]] EState GetState() const noexcept {
            return m_State;
        }

        void SetData(UserData* userData) {
            m_UserData.reset(userData);
        }

        template <typename T>
        [[nodiscard]] T* GetData() const noexcept {
            return m_UserData ? static_cast<T*>(m_UserData.get()) : nullptr;
        }

        [[nodiscard]] Address GetAddress() const noexcept {
            constexpr std::size_t length = 46;
            char ip[length]{};
            std::uint16_t port = m_Peer->address.port;
            (void)enet_address_get_ip(&m_Peer->address, ip, length);
            return {ip, port};
        }

        [[nodiscard]] Network* GetNetwork() noexcept {
            return m_Network;
        }

        [[nodiscard]] const Network* GetNetwork() const noexcept {
            return m_Network;
        }

        [[nodiscard]] ENetPeer* GetPeer() noexcept {
            return m_Peer;
        }

        [[nodiscard]] const ENetPeer* GetPeer() const noexcept {
            return m_Peer;
        }

    private:
        Network* m_Network;
        ENetPeer* m_Peer;
        EState m_State;
        std::unique_ptr<UserData> m_UserData;
    };
}

#endif // HELENA_NETWORK_CONNECTION_HPP
