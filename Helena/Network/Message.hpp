#ifndef HELENA_NETWORK_MESSAGE_HPP
#define HELENA_NETWORK_MESSAGE_HPP

#include <Helena/Dependencies/ENet.hpp>
#include <Helena/Platform/Platform.hpp>

#include <cstdint>
#include <cstddef>

namespace Helena::Network
{
    enum class EMessage : std::uint8_t 
    {
        Reliable,       // Reliable and sequenced packets
        Unreliable,     // Not reliable if packet size > MTU, but sequenced
        Unsequenced     // Not reliable and not sequenced
    };

    class Network;
    class Connection;
    class Message
    {
        friend class Network;
        friend class Connection;

    public:
        Message(std::uint8_t channel, ENetPacket* packet) : m_Packet{packet}, m_Channel{channel}, m_RefCounter{1} {}
        ~Message()
        {
            // ENet automatically frees memory after the call to send, so we don't need to free memory
            // I am using refcount here to delete messages we receive (they should be cleared by us)
            // You don't need to free memory yourself
            if(m_Packet && m_RefCounter == 1) {
                enet_packet_destroy(m_Packet);
            }
        }
        Message(const Message&) = delete;
        Message(Message&& other) noexcept
        {
            m_Packet = other.m_Packet;
            m_Channel = other.m_Channel;
            m_RefCounter = other.m_RefCounter;

            other.m_Packet = nullptr;
            other.m_RefCounter = 0;
        }
        Message& operator=(const Message&) = delete;
        Message& operator=(Message&& other) noexcept
        {
            m_Packet = other.m_Packet;
            m_Channel = other.m_Channel;
            m_RefCounter = other.m_RefCounter;

            other.m_Packet = nullptr;
            other.m_RefCounter = 0;

            return *this;
        }

        // If memory cannot be allocated, then it is sad, just use Valid method
        [[nodiscard]] static Message CreateMessage(std::uint8_t channel, std::size_t capacity, EMessage flag)
        {
            HELENA_ASSERT(capacity, "Size cannot be 0");

            ENetPacket* packet = (ENetPacket*)enet_malloc(sizeof(ENetPacket) + capacity);
            if(!packet) {
                return Message{channel, nullptr};
            }

            switch(flag)
            {
                case EMessage::Reliable:    packet->flags = ENetPacketFlag::ENET_PACKET_FLAG_RELIABLE; break;
                case EMessage::Unreliable:  packet->flags = ENetPacketFlag::ENET_PACKET_FLAG_UNRELIABLE_FRAGMENTED; break;
                case EMessage::Unsequenced: packet->flags = ENetPacketFlag::ENET_PACKET_FLAG_UNSEQUENCED; break;
            }

            packet->data = (std::uint8_t*)packet + sizeof(ENetPacket);
            packet->referenceCount = 0;
            packet->dataLength = 0;
            packet->freeCallback = nullptr;
            packet->userData = nullptr;

            return Message{channel, packet};
        }

        [[nodiscard]] static Message CreateMessage(Message& msg, std::uint8_t channel, std::size_t capacity, EMessage flag)
        {
            HELENA_ASSERT(capacity, "Size cannot be 0");
            HELENA_ASSERT(msg.m_RefCounter > 1, "You do not own this message after call send");

            if(msg.m_RefCounter > 1 || msg.GetCapacity() < capacity) {
                return CreateMessage(channel, capacity, flag);
            }

            ENetPacket* packet = msg.m_Packet;
            switch(flag)
            {
                case EMessage::Reliable:    packet->flags = ENetPacketFlag::ENET_PACKET_FLAG_RELIABLE; break;
                case EMessage::Unreliable:  packet->flags = ENetPacketFlag::ENET_PACKET_FLAG_UNRELIABLE_FRAGMENTED; break;
                case EMessage::Unsequenced: packet->flags = ENetPacketFlag::ENET_PACKET_FLAG_UNSEQUENCED; break;
            }

            packet->referenceCount = 0;
            packet->dataLength = 0;
            packet->freeCallback = nullptr;

            return Message{std::move(msg)};
        }

        void SetSize(std::size_t size) noexcept
        {
            HELENA_ASSERT(size <= GetCapacity(), "Buffer overflowed");
            HELENA_ASSERT(size <= (std::numeric_limits<std::uint32_t>::max)(), "Size > uint32_t max");

            if(size <= GetCapacity()) {
                m_Packet->dataLength = static_cast<std::uint32_t>(size);
            } else {
                m_Packet->dataLength = 0;
            }
        }

        [[nodiscard]] std::uint32_t GetSize() const noexcept {
            return m_Packet->dataLength;
        }

        [[nodiscard]] std::size_t GetCapacity() const noexcept
        {
        #if defined(HELENA_PLATFORM_WIN)
            return m_Packet ? _msize(m_Packet) - sizeof(ENetPacket) : 0;
        #elif defined(HELENA_PLATFORM_LINUX)
            return m_Packet ? malloc_usable_size(m_Packet) - sizeof(ENetPacket) : 0;
        #else
            #error Unknown platform
        #endif
        }

        [[nodiscard]] std::uint8_t* GetBuffer() const noexcept {
            return m_Packet ? m_Packet->data : nullptr;
        }

        [[nodiscard]] std::uint8_t GetChannel() const noexcept {
            return m_Channel;
        }

        [[nodiscard]] EMessage GetFlag() const noexcept
        {
            if(m_Packet->flags & ENetPacketFlag::ENET_PACKET_FLAG_RELIABLE) {
                return EMessage::Reliable;
            } else if(m_Packet->flags & ENetPacketFlag::ENET_PACKET_FLAG_UNRELIABLE_FRAGMENTED) {
                return EMessage::Unreliable;
            } else {
                return EMessage::Unsequenced;
            }
        }

        // This method is for recv messages only and guarantees the validity of the connection.
        [[nodiscard]] Connection& GetConnection() const noexcept {
            HELENA_ASSERT(m_Packet && m_Packet->userData, "Current method only for received msg");
            return *static_cast<Connection*>(m_Packet->userData);
        }

        // But I still decided to add an validate method \(*_*)/
        [[nodiscard]] bool HasConnection() const noexcept {
            return m_Packet->userData;
        }

        // If memory cannot be allocated, then it is sad
        [[nodiscard]] bool Valid() const noexcept {
            return m_Packet;
        }

    private:
        ENetPacket* m_Packet;
        std::uint8_t m_Channel;
        mutable std::uint8_t m_RefCounter;
    };
}

#endif // HELENA_NETWORK_MESSAGE_HPP
