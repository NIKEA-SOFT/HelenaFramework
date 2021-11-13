#ifndef HELENA_TYPES_TIMESPAN_HPP
#define HELENA_TYPES_TIMESPAN_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Util/Cast.hpp>

#include <stdexcept>

namespace Helena::Types
{
    class TimeSpan
    {
        static constexpr std::int64_t m_TicksPerMilliseconds    = 10000LL;
        static constexpr std::int64_t m_TicksPerSeconds         = 1000LL * m_TicksPerMilliseconds;
        static constexpr std::int64_t m_TicksPerMinutes         = 60LL * m_TicksPerSeconds;
        static constexpr std::int64_t m_TicksPerHours           = 60LL * m_TicksPerMinutes;
        static constexpr std::int64_t m_TicksPerDays            = 24LL * m_TicksPerHours;

        static constexpr auto m_DaysPerMonth                    = std::array{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

    public:
        explicit constexpr TimeSpan() noexcept : m_Ticks{} {};
        explicit constexpr TimeSpan(std::int64_t ticks) : m_Ticks{ticks} {}

        [[nodiscard]] static constexpr TimeSpan FromMilliseconds(double milliseconds) noexcept {
            return TimeSpan{static_cast<std::int64_t>(milliseconds * static_cast<double>(m_TicksPerMilliseconds))};
        }

        [[nodiscard]] static constexpr TimeSpan FromSeconds(double seconds) noexcept {
            return TimeSpan{ static_cast<std::int64_t>(seconds * static_cast<double>(m_TicksPerSeconds)) };
        }

        [[nodiscard]] static constexpr TimeSpan FromMinutes(double minutes) noexcept {
            return TimeSpan{ static_cast<std::int64_t>(minutes * static_cast<double>(m_TicksPerMinutes)) };
        }

        [[nodiscard]] static constexpr TimeSpan FromHours(double hours) noexcept {
            return TimeSpan{ static_cast<std::int64_t>(hours * static_cast<double>(m_TicksPerHours)) };
        }

        [[nodiscard]] static constexpr TimeSpan FromDays(double days) noexcept {
            return TimeSpan{ static_cast<std::int64_t>(days * static_cast<double>(m_TicksPerDays)) };
        }

        [[nodiscard]] static constexpr TimeSpan FromMin() noexcept {
            return TimeSpan{ std::numeric_limits<std::int64_t>::min() };
        }

        [[nodiscard]] static constexpr TimeSpan FromMax() noexcept {
            return TimeSpan{ std::numeric_limits<std::int64_t>::max() };
        }

        [[nodiscard]] constexpr std::int64_t GetTicks() const noexcept {
            return m_Ticks;
        }

        [[nodiscard]] constexpr std::int64_t GetTicksAbsolute() const noexcept {
            return m_Ticks < 0 ? -m_Ticks : m_Ticks;
        }

        [[nodiscard]] constexpr std::int32_t GetMilliseconds() const noexcept {
            return static_cast<std::int32_t>((m_Ticks / m_TicksPerMilliseconds) % 1000LL);
        }

        [[nodiscard]] constexpr std::int32_t GetSeconds() const noexcept {
            return static_cast<std::int32_t>((m_Ticks / m_TicksPerSeconds) % 60LL);
        }

        [[nodiscard]] constexpr std::int32_t GetMinutes() const noexcept {
            return static_cast<std::int32_t>((m_Ticks / m_TicksPerMinutes) % 60LL);
        }

        [[nodiscard]] constexpr std::int32_t GetHours() const noexcept {
            return static_cast<std::int32_t>((m_Ticks / m_TicksPerHours) % 24LL);
        }

        [[nodiscard]] constexpr std::int32_t GetDays() const noexcept {
            return static_cast<std::int32_t>(m_Ticks / m_TicksPerDays);
        }

        [[nodiscard]] constexpr double GetTotalMilliseconds() const noexcept {
            return static_cast<double>(m_Ticks) / static_cast<double>(m_TicksPerMilliseconds);
        }

        [[nodiscard]] constexpr double GetTotalSeconds() const noexcept {
            return static_cast<double>(m_Ticks) / static_cast<double>(m_TicksPerSeconds);
        }

        [[nodiscard]] constexpr double GetTotalMinutes() const noexcept {
            return static_cast<double>(m_Ticks) / static_cast<double>(m_TicksPerMinutes);
        }

        [[nodiscard]] constexpr double GetTotalHours() const noexcept {
            return static_cast<double>(m_Ticks) / static_cast<double>(m_TicksPerHours);
        }

        [[nodiscard]] constexpr double GetTotalDays() const noexcept {
            return static_cast<double>(m_Ticks) / static_cast<double>(m_TicksPerDays);
        }

        [[nodiscard]] constexpr bool IsNull() const noexcept {
            return !m_Ticks;
        }

        [[nodiscard]] constexpr bool IsNegative() const noexcept {
            return m_Ticks < 0;
        }

        [[nodiscard]] constexpr TimeSpan operator+(const TimeSpan& other) const noexcept {
            return TimeSpan{m_Ticks + other.m_Ticks};
        }

        [[nodiscard]] constexpr TimeSpan operator-(const TimeSpan& other) const noexcept {
            return TimeSpan{m_Ticks - other.m_Ticks};
        }

        [[nodiscard]] constexpr TimeSpan operator*(const TimeSpan& other) const noexcept {
            return TimeSpan{m_Ticks * other.m_Ticks};
        }

        [[nodiscard]] constexpr TimeSpan operator/(const TimeSpan& other) const noexcept {
            if(!std::is_constant_evaluated()) {
                HELENA_ASSERT(other.m_Ticks, "Divide by zero");
            }
            return TimeSpan{m_Ticks / other.m_Ticks};
        }

        TimeSpan& operator+=(const TimeSpan& other) noexcept {
            m_Ticks += other.m_Ticks;
            return *this;
        }

        TimeSpan& operator-=(const TimeSpan& other) noexcept {
            m_Ticks -= other.m_Ticks;
            return *this;
        }

        TimeSpan& operator*=(const TimeSpan& other) noexcept {
            m_Ticks *= other.m_Ticks;
            return *this;
        }

        TimeSpan& operator/=(const TimeSpan& other) noexcept {
            HELENA_ASSERT(other.m_Ticks, "Divide by zero");
            m_Ticks /= other.m_Ticks;
            return *this;
        }

        [[nodiscard]] constexpr auto operator<=>(const TimeSpan&) const noexcept = default;

    private:
        std::int64_t m_Ticks;
    };
}

#endif // HELENA_TYPES_TIMESPAN_HPP
