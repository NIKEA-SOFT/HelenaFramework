#ifndef HELENA_TYPES_TIMEDATA_HPP
#define HELENA_TYPES_TIMEDATA_HPP

#include <Helena/Types/Format.hpp>
#include <Helena/Types/TimeSpan.hpp>
#include <Helena/Util/Cast.hpp>

#include <array>
#include <cmath>

namespace Helena::Types
{
    class DateTime 
    {
        static constexpr std::int64_t m_TicksPerMilliseconds    = 10000LL;
        static constexpr std::int64_t m_TicksPerSeconds         = 1000LL * m_TicksPerMilliseconds;
        static constexpr std::int64_t m_TicksPerMinutes         = 60LL * m_TicksPerSeconds;
        static constexpr std::int64_t m_TicksPerHours           = 60LL * m_TicksPerMinutes;
        static constexpr std::int64_t m_TicksPerDays            = 24LL * m_TicksPerHours;

        static constexpr auto m_DaysPerMonth                    = std::array{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

    public:
        enum class EMonths {
            January = 1,
            February,
            March,
            April,
            May,
            June,
            July,
            August,
            September,
            October,
            November,
            December
        };

        enum class EDaysOfWeek {
            Monday, 
            Tuesday, 
            Wednesday, 
            Thursday, 
            Friday, 
            Saturday, 
            Sunday
        };

        constexpr DateTime() noexcept = default;
        constexpr ~DateTime() = default;
        constexpr DateTime(const DateTime&) noexcept = default;
        constexpr DateTime(DateTime&&) noexcept = default;
        constexpr DateTime(std::int64_t ticks) : m_Ticks{ticks} {}
        constexpr DateTime(std::int32_t year, std::int32_t month, std::int32_t day, 
            std::int32_t hour = 0, std::int32_t minute = 0, std::int32_t second = 0, std::int32_t ms = 0) {
            m_Ticks = DateToTicks(year, month, day) + TimeToTicks(hour, minute, second) + ms * m_TicksPerMilliseconds;
        }

        constexpr DateTime& operator=(const DateTime&) = default;
        constexpr DateTime& operator=(DateTime&&) noexcept = default;

        //[[nodiscard]] static std::int64_t NowTickTime() noexcept {
        //    return FromTickTime().GetTicks();
        //}

        //[[nodiscard]] static std::int64_t NowUTCTime() noexcept {
        //    return FromUTCTime().GetTicks();
        //}

        //[[nodiscard]] static std::int64_t NowLocalTime() noexcept {
        //    return FromLocalTime().GetTicks();
        //}

        [[nodiscard]] static DateTime FromTickTime() {
            return FromTimeStamp(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
        }

        [[nodiscard]] static DateTime FromUTCTime() {
            const auto time_zone = std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now());
            return FromTimeStamp(std::chrono::duration_cast<std::chrono::seconds>(time_zone.get_sys_time().time_since_epoch()).count());
        }

        [[nodiscard]] static DateTime FromLocalTime() {
            const auto time_zone = std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now());
            return FromTimeStamp(std::chrono::duration_cast<std::chrono::seconds>(time_zone.get_local_time().time_since_epoch()).count());
        }

        [[nodiscard]] static constexpr DateTime FromTimeStamp(std::int64_t seconds) noexcept {
            return DateTime(DateToTicks(1970, 1, 1) + seconds * m_TicksPerSeconds);
        }

        [[nodiscard]] static constexpr DateTime FromMilliseconds(std::int64_t milliseconds) noexcept {
            return DateTime(DateToTicks(1970, 1, 1) + milliseconds * m_TicksPerMilliseconds);
        }

        // %D - Day
        // %M - Month
        // %Y - Year
        // %h - hours
        // %m - minutes
        // %s - seconds
        // %ms - milliseconds
        // Currently this function not constexpr becaus Util::Cast use from_chars
        [[nodiscard]] static DateTime FromString(std::string_view format, std::string_view time) noexcept
        {
            if(format.empty() || time.empty()) {
                return DateTime{};
            }

            bool hasError{};

            std::int32_t day{};
            std::int32_t month{};
            std::int32_t year{};
            std::int32_t hour{};
            std::int32_t minutes{};
            std::int32_t seconds{};
            std::int32_t milliseconds{};

            std::size_t offsetFormat{};
            std::size_t offsetTime{};

            const auto fnParse = [](std::string_view buffer, std::size_t& offset, std::size_t read_length, std::int32_t& out) -> bool 
            {
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

                    switch(format[offsetFormat]) 
                    {
                        case 'D': hasError = fnParse(time, offsetTime, 2uLL, day); break;
                        case 'M': hasError = fnParse(time, offsetTime, 2uLL, month); break;
                        case 'Y': hasError = fnParse(time, offsetTime, 4uLL, year); break;
                        case 'h': hasError = fnParse(time, offsetTime, 2uLL, hour); break;
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
                    return DateTime{};
                }
            }



            const auto ticks = DateToTicks(year, month, day) + TimeToTicks(hour, minutes, seconds, milliseconds);
            return DateTime{ticks};
        }

        [[nodiscard]] constexpr bool IsNull() const noexcept {
            return !m_Ticks;
        }

        [[nodiscard]] constexpr bool IsNegative() const noexcept {
            return m_Ticks < 0;
        }

        [[nodiscard]] constexpr double GetJulianDay() const noexcept {
            return static_cast<double>(m_Ticks) / static_cast<double>(m_TicksPerDays) + 1721425.5;
        }

        [[nodiscard]] constexpr std::int64_t GetTicks() const noexcept {
            return m_Ticks;
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

        [[nodiscard]] constexpr std::int32_t GetHour() const noexcept {
            return static_cast<std::int32_t>((m_Ticks / m_TicksPerHours) % 24LL);
        }

        [[nodiscard]] constexpr std::int32_t GetDay() const noexcept {
            std::int32_t year, month, day;
            GetDate(year, month, day);
            return day;
        }

        [[nodiscard]] constexpr std::int32_t GetMonth() const noexcept {
            std::int32_t year, month, day;
            GetDate(year, month, day);
            return month;
        }

        [[nodiscard]] constexpr std::int32_t GetYear() const noexcept {
            std::int32_t year, month, day;
            GetDate(year, month, day);
            return year;
        }

        [[nodiscard]] constexpr std::int64_t GetTimeStamp() const noexcept {
            return (m_Ticks - DateToTicks(1970, 1, 1)) / m_TicksPerSeconds;
        }

        [[nodiscard]] constexpr EDaysOfWeek GetDayOfWeek() const noexcept {
            return static_cast<EDaysOfWeek>((m_Ticks / m_TicksPerDays) % 7);
        }

        [[nodiscard]] constexpr DateTime GetDate() const noexcept {
            return DateTime(m_Ticks - m_Ticks % m_TicksPerDays);
        }

        constexpr void GetDate(std::int32_t& year, std::int32_t& month, std::int32_t& day) const noexcept {
            std::int32_t l = static_cast<std::int32_t>(GetJulianDay() + 0.5) + 68569;
            const std::int32_t n = 4 * l / 146097;
            l = l - (146097 * n + 3) / 4;
            std::int32_t i = 4000 * (l + 1) / 1461001;
            l = l - 1461 * i / 4 + 31;
            std::int32_t j = 80 * l / 2447;
            const std::int32_t k = l - 2447 * j / 80;
            l = j / 11;
            j = j + 2 - 12 * l;
            i = 100 * (n - 49) + i + l;
            year = i; month = j; day = k;
        }

        [[nodiscard]] static constexpr std::int32_t GetDaysInMonth(std::int32_t year, std::int32_t month) noexcept {
            HELENA_ASSERT(month >= 1 && month <= 12);
            return m_DaysPerMonth[month] - m_DaysPerMonth[month - 1LL] - (IsLeapYear(year) ? 1 : 0);
        }

        [[nodiscard]] static constexpr std::int32_t GetDaysInYear(std::int32_t year) noexcept {
            HELENA_ASSERT(year >= 1 && year <= 9999);
            return IsLeapYear(year) ? 366 : 365;
        }

        [[nodiscard]] static constexpr bool IsLeapYear(std::int32_t year) noexcept {
            HELENA_ASSERT(year >= 1 && year <= 9999);
            return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
        }

        [[nodiscard]] static constexpr std::int64_t DateToTicks(std::int32_t year, std::int32_t month, std::int32_t day) noexcept {
            HELENA_ASSERT(day >= 1 && day <= GetDaysInMonth(year, month));
            --year;
            --month;
            return (year * 365LL + year / 4LL - year / 100LL + year / 400LL + m_DaysPerMonth[month] + static_cast<std::int32_t>(IsLeapYear(year)) + day - 1LL) * m_TicksPerDays;
        }

        [[nodiscard]] static constexpr std::int64_t TimeToTicks(std::int32_t hour, std::int32_t minute, std::int32_t second, std::int32_t millisecond = 0) noexcept {
            HELENA_ASSERT(hour >= 0 && hour <= 23);
            HELENA_ASSERT(minute >= 0 && minute <= 59);
            HELENA_ASSERT(second >= 0 && second <= 59);
            return (hour * 3600LL + minute * 60LL + second) * m_TicksPerSeconds + millisecond * m_TicksPerMilliseconds;
        }

        constexpr DateTime operator-(const DateTime& other) const noexcept {
            return DateTime(m_Ticks - other.m_Ticks);
        }

        constexpr DateTime operator+(const DateTime& other) const noexcept {
            return DateTime(m_Ticks - other.m_Ticks);
        }

        DateTime& operator-=(const DateTime& other) noexcept {
            m_Ticks -= other.m_Ticks;
            return *this;
        }

        DateTime& operator+=(const DateTime& other) noexcept {
            m_Ticks += other.m_Ticks;
            return *this;
        }

        [[nodiscard]] constexpr auto operator<=>(const DateTime&) const noexcept = default;

    private:
        std::int64_t m_Ticks {};
    };
}

#endif // HELENA_TYPES_TIMESPAN_HPP
