#ifndef HELENA_TYPES_TIMESPAN_HPP
#define HELENA_TYPES_TIMESPAN_HPP

#include <Helena/Platform/Assert.hpp>

namespace Helena::Types
{
    class TimeSpan
    {
        static constexpr std::int64_t m_TicksPerMS = 10000LL;
        static constexpr std::int64_t m_TicksPerSeconds = 1000LL * m_TicksPerMS;
        static constexpr std::int64_t m_TicksPerMinutes = 60LL * m_TicksPerSeconds;
        static constexpr std::int64_t m_TicksPerHours = 60LL * m_TicksPerMinutes;
        static constexpr std::int64_t m_TicksPerDays = 24LL * m_TicksPerHours;

    public:
        explicit constexpr TimeSpan() noexcept : m_Ticks{} {};
        explicit constexpr TimeSpan(std::int64_t ticks) : m_Ticks{ticks} {}
        explicit constexpr TimeSpan(std::int64_t hours, std::int64_t min, std::int64_t sec, std::int64_t ms)
            : m_Ticks{hours * m_TicksPerHours
            + min * m_TicksPerMinutes
            + sec * m_TicksPerSeconds
            + ms * m_TicksPerMS} {}
        explicit constexpr TimeSpan(std::int64_t days, std::int64_t hours, std::int64_t min, std::int64_t sec, std::int64_t ms)
            : m_Ticks{days * m_TicksPerDays
            + hours * m_TicksPerHours
            + min * m_TicksPerMinutes
            + sec * m_TicksPerSeconds
            + ms * m_TicksPerMS} {}

        [[nodiscard]] static constexpr TimeSpan FromTimeStamp(std::int64_t seconds) noexcept {
            return TimeSpan{seconds * m_TicksPerSeconds};
        }

        [[nodiscard]] static constexpr TimeSpan FromMS(std::int64_t milliseconds) noexcept {
            return TimeSpan{milliseconds * m_TicksPerMS};
        }

        [[nodiscard]] static constexpr TimeSpan FromSeconds(std::int64_t seconds) noexcept {
            return TimeSpan{seconds * m_TicksPerSeconds};
        }

        [[nodiscard]] static constexpr TimeSpan FromMinutes(std::int64_t minutes) noexcept {
            return TimeSpan{minutes * m_TicksPerMinutes};
        }

        [[nodiscard]] static constexpr TimeSpan FromHours(std::int64_t hours) noexcept {
            return TimeSpan{hours * m_TicksPerHours};
        }

        [[nodiscard]] static constexpr TimeSpan FromDays(std::int64_t days) noexcept {
            return TimeSpan{days * m_TicksPerDays};
        }

        [[nodiscard]] constexpr std::int64_t GetTicks() const noexcept {
            return m_Ticks;
        }

        [[nodiscard]] constexpr std::int64_t GetTicksAbsolute() const noexcept {
            return m_Ticks < 0 ? -m_Ticks : m_Ticks;
        }

        [[nodiscard]] constexpr std::int32_t GetMS() const noexcept {
            return static_cast<std::int32_t>((m_Ticks / m_TicksPerMS) % 1000LL);
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

        [[nodiscard]] constexpr double GetTotalMS() const noexcept {
            return static_cast<double>(m_Ticks) / static_cast<double>(m_TicksPerMS);
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

        constexpr auto operator<=>(const TimeSpan&) const noexcept = default;

    private:
        std::int64_t m_Ticks;
    };
}

#endif // HELENA_TYPES_TIMESPAN_HPP