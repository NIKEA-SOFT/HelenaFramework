#ifndef HELENA_TYPES_TIMESPAN_HPP
#define HELENA_TYPES_TIMESPAN_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Util/Cast.hpp>
#include <Helena/Util/Format.hpp>

#include <charconv>
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

    public:
        explicit constexpr TimeSpan() : m_Ticks{} {};
        explicit constexpr TimeSpan(std::int64_t ticks) : m_Ticks{ticks} {}

        [[nodiscard]] static constexpr TimeSpan FromMilliseconds(double milliseconds) noexcept {
            return TimeSpan{static_cast<std::int64_t>(milliseconds * static_cast<double>(m_TicksPerMilliseconds))};
        }

        [[nodiscard]] static constexpr TimeSpan FromSeconds(double seconds) noexcept {
            return TimeSpan{static_cast<std::int64_t>(seconds * static_cast<double>(m_TicksPerSeconds))};
        }

        [[nodiscard]] static constexpr TimeSpan FromMinutes(double minutes) noexcept {
            return TimeSpan{static_cast<std::int64_t>(minutes * static_cast<double>(m_TicksPerMinutes))};
        }

        [[nodiscard]] static constexpr TimeSpan FromHours(double hours) noexcept {
            return TimeSpan{static_cast<std::int64_t>(hours * static_cast<double>(m_TicksPerHours))};
        }

        [[nodiscard]] static constexpr TimeSpan FromDays(double days) noexcept {
            return TimeSpan{static_cast<std::int64_t>(days * static_cast<double>(m_TicksPerDays))};
        }

        [[nodiscard]] static constexpr TimeSpan FromMin() noexcept {
            return TimeSpan{std::numeric_limits<std::int64_t>::min()};
        }

        [[nodiscard]] static constexpr TimeSpan FromMax() noexcept {
            return TimeSpan{std::numeric_limits<std::int64_t>::max()};
        }

        // %D - Day
        // %M - Month
        // %Y - Year
        // %h - hours
        // %m - minutes
        // %s - seconds
        // %ms - milliseconds
        [[nodiscard]] static constexpr TimeSpan FromString(std::string_view format, std::string_view time) noexcept 
        {
            if(format.empty() || time.empty()) {
                return TimeSpan{};
            }

            bool hasError {};

            std::int32_t day{};
            std::int32_t month{};
            std::int32_t year{};
            std::int32_t minutes{};
            std::int32_t seconds{};
            std::int32_t milliseconds{};

            std::size_t offsetFormat{};
            std::size_t offsetTime{};

            constexpr auto fnParse = [](std::string_view buffer, std::size_t& offset, std::size_t read_length, std::int32_t& out) -> bool 
            {
                // Take string with fixed size length from buffer
                const auto data = buffer.substr(offset, read_length);

                // Buffer can be small, need compare for check
                if(data.size() == read_length)
                {
                    // Cast string to int
                    const auto value = Util::Cast<std::int32_t>(data);

                    // Check cast result
                    if(value.has_value()) 
                    {
                        // Write casted value to out variable
                        out = value.value();

                        // Add size to offset for parse other data in next time
                        offset += data.size();

                        return true;
                    }
                    
                    HELENA_MSG_EXCEPTION("Cast data: \"{}\" failed", data);
                } else 
                    HELENA_MSG_EXCEPTION("Parse data: \"{}\" failed, data size less than read_length", data);

                return false;
            };

            while(true) 
            {
                if(offsetFormat >= format.size()) {
                    break;
                }

                if(format[offsetFormat] == '%')
                {
                    ++offsetFormat;
                    if(offsetFormat >= format.size()) {
                        break;
                    }

                    switch(format[offsetFormat])
                    {
                        case 'D': {
                            hasError = fnParse(time, offsetTime, 4uLL, year);
                        } break;

                        case 'M': {

                        } break;

                        case 'Y': {

                        } break;

                        case 'h': {

                        } break;

                        case 'm': {


                        } break;

                        default: {
                            HELENA_ASSERT(false, "Format: \"{}\" incorrect, time: \"{}\"", format, time);
                            return TimeSpan{};
                        }
                    }
                }
            }

            return timespan;
        }

        [[nodiscard]] constexpr double GetTotalMicroseconds() const noexcept;
        //[[nodiscard]] constexpr double GetTotalMilliseconds() const noexcept;
        //[[nodiscard]] constexpr double GetTotalSeconds() const noexcept;
        //[[nodiscard]] constexpr double GetTotalMinutes() const noexcept;
        //[[nodiscard]] constexpr double GetTotalHours() const noexcept;
        //[[nodiscard]] constexpr double GetTotalDays() const noexcept;

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
            return static_cast<std::int32_t>((m_Ticks / m_TicksPerSeconds) % 60LL);
        }

        [[nodiscard]] constexpr std::int32_t GetHours() const noexcept {
            return static_cast<std::int32_t>((m_Ticks / m_TicksPerHours) % 24LL);
        }

        [[nodiscard]] constexpr std::int32_t GetDays() const noexcept {
            return static_cast<std::int32_t>(m_Ticks / m_TicksPerDays);
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
