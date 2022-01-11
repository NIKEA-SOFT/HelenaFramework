#ifndef HELENA_NETWORK_NETWORK_HPP
#define HELENA_NETWORK_NETWORK_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Network/Address.hpp>
#include <Helena/Network/Message.hpp>
#include <Helena/Network/Event.hpp>
#include <Helena/Network/Config.hpp>
#include <Helena/Network/Connection.hpp>

#include <functional>
#include <unordered_map>

namespace Helena::Network
{
    enum class EType {
        Server,
        Client
    };

    class Network
    {
        using NetMsgCB = std::function<void(Message)>;
        using NetEventCB = std::function<void(Event)>;

        [[nodiscard]] bool Server(Address address, Config config)
        {
            if(m_Host) {
                HELENA_MSG_ERROR("Network server already started!");
                return false;
            }

            if(!m_CallbackEvent) {
                HELENA_MSG_ERROR("Network event callback not registered!");
                return false;
            }

            if(!m_CallbackMsg) {
                HELENA_MSG_ERROR("Network msg callback not registered!");
                return false;
            }

            ENetAddress addr{}; addr.port = address.GetPort();
            if(enet_address_set_ip(&addr, address.GetIP().GetData())) {
                HELENA_MSG_ERROR("Network server set host ip: {}, port: {} failed!", address.GetIP(), address.GetPort());
                return false;
            }

            const auto host = enet_host_create(&addr, config.m_MaxPeers, config.m_MaxChannels, config.m_BandwidthIn, config.m_BandwidthOut);
            if(!host) {
                HELENA_MSG_ERROR("Network create host with ip: {}, port: {}, peers: {}, channels: {} failed!",
                    address.GetIP(), address.GetPort(), config.m_MaxPeers, config.m_MaxChannels);
                return false;
            }

            m_Host = host;
            return true;
        }

        [[nodiscard]] bool Client(Address address, Config config, std::uint32_t data = 0)
        {
            if(!m_CallbackEvent) {
                HELENA_MSG_ERROR("Network event callback not registered!");
                return false;
            }

            if(!m_CallbackMsg) {
                HELENA_MSG_ERROR("Network msg callback not registered!");
                return false;
            }

            if(!m_Host)
            {
                const auto host = enet_host_create(nullptr, config.m_MaxPeers, 1, config.m_BandwidthIn, config.m_BandwidthOut);
                if(!host) {
                    HELENA_MSG_ERROR("Network create host with ip: {}, port: {}, peers: {}, channels: {} failed!",
                        address.GetIP(), address.GetPort(), config.m_MaxPeers, config.m_MaxChannels);
                    return false;
                }

                m_Host = host;
            }

            ENetAddress addr{}; addr.port = address.GetPort();
            if(enet_address_set_ip(&addr, address.GetIP().GetData())) {
                HELENA_MSG_ERROR("Network client set host ip: {}, port: {} failed!", address.GetIP(), address.GetPort());
                return false;
            }

            const auto peer = enet_host_connect(m_Host, &addr, config.m_MaxChannels, data);
            if(!peer) {
                HELENA_MSG_ERROR("Network connect to ip: {}, port: {} failed!", address.GetIP(), address.GetPort());
                return false;
            }

            return true;
        }

    public:
        Network() : m_Host{} {
            [[maybe_unused]] bool enet_init = !enet_initialize();
            HELENA_ASSERT(enet_init, "Initialize network failed");
        }
        virtual ~Network() 
        {
            if(m_Host) {
                enet_host_flush(m_Host);
                enet_host_destroy(m_Host);
                m_Connections.clear();
                enet_deinitialize();
            }
        }
        Network(const Network&) = delete;
        Network(Network&&) noexcept = delete;
        Network& operator=(const Network&) = delete;
        Network& operator=(Network&&) noexcept = delete;

        [[nodiscard]] bool Initialize(EType type, Address address, Config config, std::uint32_t data = 0) {
            return type == EType::Server ? Server(address, config) : Client(address, config, data);
        }

        [[nodiscard]] bool State() {
            return m_Host;
        }

        void Shutdown()
        {
            if(m_Host) {
                enet_host_flush(m_Host);
                enet_host_destroy(m_Host);
                m_Connections.clear();
            }
        }

        void RegMsgCB(NetMsgCB&& cb) {
            m_CallbackMsg = std::move(cb);
        }

        void RegEventCB(NetEventCB&& cb) {
            m_CallbackEvent = std::move(cb);
        }

        [[nodiscard]] bool Broadcast(const Message& message) const
        {
            HELENA_ASSERT(message.m_Packet, "Packet is nullptr");
            if(!message.m_Packet) {
                return false;
            }

            HELENA_ASSERT(message.m_Packet->dataLength, "Packet is nullptr");
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

            enet_host_broadcast(m_Host, message.m_Channel, message.m_Packet);
            return true;
        }

        template <typename Func>
        void Each(Func&& fn)
        {
            for(auto& [key, value] : m_Connections) {
                fn(value);
            }
        }

        template <typename Func>
        void Each(Func&& fn) const
        {
            for(const auto& [key, value] : m_Connections) {
                fn(value);
            }
        }

        [[nodiscard]] Connection* GetConnection(std::uint32_t id) {
            const auto it = m_Connections.find(id);
            return it == m_Connections.cend() ? nullptr : &it->second;
        }

        [[nodiscard]] const Connection* GetConnection(std::uint32_t id) const {
            const auto it = m_Connections.find(id);
            return it == m_Connections.cend() ? nullptr : &it->second;
        }

        void Update()
        {
            if(m_Host)
            {
                while(true)
                {
                    ENetEvent event{};
                    if(enet_host_check_events(m_Host, &event) <= 0)
                    {
                        if(enet_host_service(m_Host, &event, 0) <= 0) {
                            return;
                        }
                    }

                    switch(event.type)
                    {
                        case ENET_EVENT_TYPE_NONE: break;
                        case ENET_EVENT_TYPE_CONNECT:
                        {
                            if(!m_CallbackEvent) {
                                enet_peer_disconnect_now(event.peer, 0);
                                break;
                            }

                            const auto& pair = m_Connections.emplace(std::piecewise_construct,
                                std::forward_as_tuple(enet_peer_get_id(event.peer)),
                                std::forward_as_tuple(this, event.peer, EState::Connected));

                            event.peer->data = &pair.first->second;
                            m_CallbackEvent({&pair.first->second, EEvent::Connect});
                        } break;
                        case ENET_EVENT_TYPE_DISCONNECT:
                        {
                            const auto connection = static_cast<Connection*>(event.peer->data);
                            if(m_CallbackEvent && connection) {
                                m_CallbackEvent({connection, EEvent::Disconnect});
                                m_Connections.erase(enet_peer_get_id(event.peer));
                            }
                        } break;
                        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                        {
                            const auto connection = static_cast<Connection*>(event.peer->data);
                            if(m_CallbackEvent && connection) {
                                m_CallbackEvent({connection, EEvent::Timeout});
                                m_Connections.erase(enet_peer_get_id(event.peer));
                            }
                        } break;
                        case ENET_EVENT_TYPE_RECEIVE:
                        {
                            const auto connection = static_cast<Connection*>(event.peer->data);
                            if(m_CallbackMsg && connection) {
                                event.packet->userData = connection;
                                m_CallbackMsg({event.channelID, event.packet});
                            } else {
                                enet_packet_destroy(event.packet);
                            }
                        } break;
                    }
                }
            }
        }

    protected:
        std::unordered_map<std::uint32_t, Connection> m_Connections;

        NetMsgCB    m_CallbackMsg;
        NetEventCB  m_CallbackEvent;

        ENetHost*   m_Host;
    };
}

#endif // HELENA_NETWORK_NETWORK_HPP
