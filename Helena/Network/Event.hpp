#ifndef HELENA_NETWORK_EVENT_HPP
#define HELENA_NETWORK_EVENT_HPP

#include <Helena/Network/State.hpp>

namespace Helena::Network
{
    enum class EEvent : std::uint8_t {
        Connect,
        Disconnect,
        Timeout
    };

    class Connection;

    class Event 
    {
    public:
        Event(Connection* connection, EEvent type) : m_Connection{connection}, m_Type{type} {}
        ~Event() = default;
        Event(const Event&) = default;
        Event(Event&&) noexcept = default;
        Event& operator=(const Event&) = default;
        Event& operator=(Event&&) noexcept = default;

        [[nodiscard]] Connection& GetConnection() const noexcept {
            return *m_Connection;
        }

        [[nodiscard]] EEvent GetType() const noexcept {
            return m_Type;
        }

    private:
        Connection* m_Connection;
        EEvent m_Type;
    };
}

#endif // HELENA_NETWORK_EVENT_HPP
