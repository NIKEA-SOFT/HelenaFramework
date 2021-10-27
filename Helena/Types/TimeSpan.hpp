#ifndef HELENA_TYPES_TIMESPAN_HPP
#define HELENA_TYPES_TIMESPAN_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Util/Cast.hpp>

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

        // %D - Day
        // %M - Month
        // %Y - Year
        // %h - hours
        // %m - minutes
        // %s - seconds
        // Currently this function not constexpr becaus Util::Cast use from_chars
        [[nodiscard]] static TimeSpan FromString(std::string_view format, std::string_view time) noexcept
        {
            if(format.empty() || time.empty()) {
                return TimeSpan{};
            }

            bool hasError{};

            std::int32_t day{};
            std::int32_t month{};
            std::int32_t year{};
            std::int32_t hours{};
            std::int32_t minutes{};
            std::int32_t seconds{};
            std::int32_t milliseconds{};

            std::size_t offsetFormat{};
            std::size_t offsetTime{};

            constexpr auto fnParse = [](std::string_view buffer, std::size_t& offset, std::size_t read_length, std::int32_t& out) -> bool {
                // Take string with fixed size length from buffer
                const auto data = buffer.substr(offset, read_length);

                // Buffer can be small, need compare for check
                if(data.size() == read_length) {
                    // Cast string to int
                    const auto value = Util::Cast<std::int32_t>(data);

                    // Check cast result
                    if(value.has_value()) {
                        // Write casted value to out variable
                        out = value.value();

                        // Add size to offset for parse other data in next time
                        offset += data.size();

                        return false;
                    }

                    HELENA_MSG_EXCEPTION("Cast data: \"{}\" failed", data);
                } else
                    HELENA_MSG_EXCEPTION("Parse data: \"{}\" failed, data size less than read_length", data);

                return true;
            };

            // 10:15:32.680
            // %h:%m:%s.%ms
            while(true) 
            {
                if(offsetFormat >= format.size()) {
                    break;
                }

                if(format[offsetFormat++] == '%') 
                {
                    if(offsetFormat >= format.size()) {
                        hasError = true;
                        continue;
                    }

                    if(offsetTime >= time.size()) {
                        hasError = true;
                        continue;
                    }

                    switch(format[offsetFormat]) {
                        case 'D': hasError = fnParse(time, offsetTime, 2uLL, day); break;
                        case 'M': hasError = fnParse(time, offsetTime, 2uLL, month); break;
                        case 'Y': hasError = fnParse(time, offsetTime, 4uLL, year); break;
                        case 'h': hasError = fnParse(time, offsetTime, 2uLL, hours); break;
                        case 'm':
                            if(format[offsetFormat + 1] == 's') {
                                ++offsetFormat;
                                hasError = fnParse(time, offsetTime, 3uLL, milliseconds);
                            } else {
                                hasError = fnParse(time, offsetTime, 2uLL, minutes);
                            }
                        break;
                        case 's': hasError = fnParse(time, offsetTime, 2uLL, seconds); break;

                        default: {
                            HELENA_ASSERT(false, "Format: \"{}\" incorrect, time: \"{}\"", format, time);
                            hasError = true;
                        }
                    }

                    ++offsetFormat;
                } else {
                    ++offsetTime;
                }

                if(hasError) {
                    return TimeSpan{};
                }
            }



            const bool isLeap = (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));
            const std::int32_t daysInMonth = 
                                (month == 2 ? (isLeap ? 29 : 28) : ((month == 4 || month == 6 || month == 9 || month == 11) ? 30 : 31));

            if(month > 12 || day > daysInMonth || hours > 24 || minutes > 60 || seconds > 60 || milliseconds > 1000) {
                return TimeSpan{};
            }

            const std::int64_t ticks =  
                                (static_cast<std::int64_t>(year * (isLeap ? 366 : 365)) * m_TicksPerDays) +
                                (static_cast<std::int64_t>(month * daysInMonth) * m_TicksPerDays) +
                                (static_cast<std::int64_t>(day) * m_TicksPerDays) +
                                (static_cast<std::int64_t>(hours) * m_TicksPerHours) +
                                (static_cast<std::int64_t>(minutes) * m_TicksPerMinutes) +
                                (static_cast<std::int64_t>(seconds) * m_TicksPerSeconds) +
                                (static_cast<std::int64_t>(milliseconds * m_TicksPerMilliseconds));

            return TimeSpan{ticks};
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
