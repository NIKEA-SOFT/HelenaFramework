#ifndef HELENA_TYPES_TIMESPAN_HPP
#define HELENA_TYPES_TIMESPAN_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Util/Format.hpp>

#include <chrono>
#include <stdexcept>

namespace Helena::Types
{
    class TimeSpan
    {
    public:
        explicit constexpr TimeSpan() : m_TimeStamp{} {};
        explicit constexpr TimeSpan(std::int64_t timestamp) : m_TimeStamp{timestamp} {}

        [[nodiscard]] static constexpr TimeSpan FromTimeStamp(std::int64_t timestamp);
        [[nodiscard]] static constexpr TimeSpan FromNanoseconds(double nanoseconds);
        [[nodiscard]] static constexpr TimeSpan FromMilliseconds(double milliseconds);
        [[nodiscard]] static constexpr TimeSpan FromSeconds(double seconds);
        [[nodiscard]] static constexpr TimeSpan FromMinutes(double minutes);
        [[nodiscard]] static constexpr TimeSpan FromHours(double hours);
        [[nodiscard]] static constexpr TimeSpan FromDays(double days);
        [[nodiscard]] static constexpr TimeSpan FromString(const std::string_view time, const std::string_view format);
        //[[nodiscard]] static constexpr TimeSpan FromInfinity();
        //[[nodiscard]] static constexpr TimeSpan FromInfinityNegative();

        //[[nodiscard]] constexpr double GetTotalMicroseconds() const noexcept;
        //[[nodiscard]] constexpr double GetTotalMilliseconds() const noexcept;
        //[[nodiscard]] constexpr double GetTotalSeconds() const noexcept;
        //[[nodiscard]] constexpr double GetTotalMinutes() const noexcept;
        //[[nodiscard]] constexpr double GetTotalHours() const noexcept;
        //[[nodiscard]] constexpr double GetTotalDays() const noexcept;

        [[nodiscard]] constexpr std::int64_t GetTimeStamp() const noexcept {
            return m_TimeStamp;
        }

        [[nodiscard]] constexpr std::int32_t GetNanoseconds() const noexcept;
        [[nodiscard]] constexpr std::int32_t GetMicroseconds() const noexcept;
        [[nodiscard]] constexpr std::int32_t GetMilliseconds() const noexcept;
        [[nodiscard]] constexpr std::int32_t GetSeconds() const noexcept;
        [[nodiscard]] constexpr std::int32_t GetMinutes() const noexcept;
        [[nodiscard]] constexpr std::int32_t GetHours() const noexcept;
        [[nodiscard]] constexpr std::int32_t GetDays() const noexcept;

        [[nodiscard]] constexpr bool IsNull() const noexcept {
            return m_TimeStamp;
        }

        [[nodiscard]] constexpr TimeSpan operator+(const TimeSpan& other) const noexcept {
            return TimeSpan{m_TimeStamp + other.m_TimeStamp};
        }

        [[nodiscard]] constexpr TimeSpan operator-(const TimeSpan& other) const noexcept {
            return TimeSpan{m_TimeStamp - other.m_TimeStamp};
        }

        [[nodiscard]] constexpr TimeSpan operator*(const TimeSpan& other) const noexcept {
            return TimeSpan{m_TimeStamp * other.m_TimeStamp};
        }

        [[nodiscard]] constexpr TimeSpan operator/(const TimeSpan& other) const noexcept {
            HELENA_ASSERT(other.m_TimeStamp, "Divide by zero");
            return TimeSpan{m_TimeStamp / other.m_TimeStamp};
        }

        TimeSpan& operator+=(const TimeSpan& other) noexcept {
            m_TimeStamp += other.m_TimeStamp;
            return *this;
        }

        TimeSpan& operator-=(const TimeSpan& other) noexcept {
            m_TimeStamp -= other.m_TimeStamp;
            return *this;
        }

        TimeSpan& operator*=(const TimeSpan& other) noexcept {
            m_TimeStamp *= other.m_TimeStamp;
            return *this;
        }

        TimeSpan& operator/=(const TimeSpan& other) noexcept {
            HELENA_ASSERT(other.m_TimeStamp, "Divide by zero");
            m_TimeStamp /= other.m_TimeStamp;
            return *this;
        }

        [[nodiscard]] constexpr auto operator<=>(const TimeSpan&) const noexcept = default;

    private:
        std::int64_t m_TimeStamp;
    };
}

#endif // HELENA_TYPES_TIMESPAN_HPP
