#ifndef HELENA_TYPES_DATETIME_HPP
#define HELENA_TYPES_DATETIME_HPP

#include <Helena/Platform/Assert.hpp>
#include <Helena/Platform/Platform.hpp>
#include <array>
#include <ctime>
#include <charconv>
#include <chrono>

// #define HELENA_ZONED_TIME -> enable use of zoned_time forcibly

// Decided to deprecate zoned_time for the following reasons:
// Linux: temporarily not supported by compilers
// Windows: no backwards compatible with Windows 7 and 8
// If your Linux already has a compiler with zoned_time support,
// and the lack of backward compatibility on Windows 7 and 8 doesn't bother you,
// you can enable the macro HELENA_ZONED_TIME

namespace Helena::Types
{
    class DateTime
    {
        static constexpr std::int64_t m_TicksPerMilliseconds = 10000LL;
        static constexpr std::int64_t m_TicksPerSeconds = 1000LL * m_TicksPerMilliseconds;
        static constexpr std::int64_t m_TicksPerMinutes = 60LL * m_TicksPerSeconds;
        static constexpr std::int64_t m_TicksPerHours = 60LL * m_TicksPerMinutes;
        static constexpr std::int64_t m_TicksPerDays = 24LL * m_TicksPerHours;

        static constexpr auto DaysPerMonth = std::array{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
        static constexpr auto DaysInWeek = 7;
        static constexpr auto Months = 12;
    public:
        enum class EMonths : std::uint8_t {
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

        enum class EDaysOfWeek : std::uint8_t {
            Monday = 0,
            Tuesday,
            Wednesday,
            Thursday,
            Friday,
            Saturday,
            Sunday
        };

        constexpr DateTime() noexcept : m_Ticks{} {};
        constexpr DateTime(const DateTime&) noexcept = default;
        constexpr DateTime(DateTime&&) noexcept = default;
        explicit constexpr DateTime(std::int64_t ticks) : m_Ticks{ticks} {}
        explicit constexpr DateTime(std::int32_t year, std::int32_t month, std::int32_t day,
            std::int32_t hour = 0, std::int32_t minute = 0, std::int32_t second = 0, std::int32_t millisecond = 0) : m_Ticks{} {
            HELENA_ASSERT(Valid(year, month, day, hour, minute, second, millisecond));
            m_Ticks = DateToTicks(year, month, day) + TimeToTicks(hour, minute, second) + millisecond * m_TicksPerMilliseconds;
        }
        constexpr DateTime& operator=(const DateTime&) = default;
        constexpr DateTime& operator=(DateTime&&) noexcept = default;

        [[nodiscard]] static DateTime FromTickTime() {
            return FromSeconds(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
        }

        [[nodiscard]] static DateTime FromUTCTime() {
        // Decided to deprecate zoned_time for the following reasons:
        // Linux: temporarily not supported by compilers
        // Windows: no backwards compatible with Windows 7 and 8
        #if defined(HELENA_ZONED_TIME)
            const auto timeZone = std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now());
            return FromSeconds(std::chrono::duration_cast<std::chrono::seconds>(timeZone.get_sys_time().time_since_epoch()).count());
        #else
            const auto timeNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm tm;
            #if defined(HELENA_PLATFORM_WIN)
                (void)::gmtime_s(&tm, &timeNow);
            #elif defined(HELENA_PLATFORM_LINUX)
                (void)::gmtime_r(&timeNow, &tm);
            #else
                #error Unknown platform detected!
            #endif
            return DateTime{DateToTicks(1900 + tm.tm_year, ++tm.tm_mon, tm.tm_mday)
                + TimeToTicks(tm.tm_hour, tm.tm_min, tm.tm_sec)};
        #endif
        }

        [[nodiscard]] static DateTime FromLocalTime() {
        // Decided to deprecate zoned_time for the following reasons:
        // Linux: temporarily not supported by compilers
        // Windows: no backwards compatible with Windows 7 and 8
        #if defined(HELENA_ZONED_TIME)
            const auto timeZone = std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now());
            return FromSeconds(std::chrono::duration_cast<std::chrono::seconds>(timeZone.get_local_time().time_since_epoch()).count());
        #else
            const auto timeNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm tm;
            #if defined(HELENA_PLATFORM_WIN)
                (void)::localtime_s(&tm, &timeNow);
            #elif defined(HELENA_PLATFORM_LINUX)
                (void)::localtime_r(&timeNow, &tm);
            #else
                #error Unknown platform detected!
            #endif
            return DateTime{DateToTicks(1900 + tm.tm_year, ++tm.tm_mon, tm.tm_mday)
                + TimeToTicks(tm.tm_hour, tm.tm_min, tm.tm_sec)};
        #endif
        }

        [[nodiscard]] static constexpr DateTime FromSeconds(std::int64_t seconds) noexcept {
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
        // Currently this function not constexpr because used from_chars
        [[nodiscard]] static DateTime FromString(std::string_view format, std::string_view time) noexcept
        {
            if(format.empty() || time.empty()) {
                return DateTime{};
            }

            std::int32_t day{}, month{}, year{}, hour{}, minutes{}, seconds{}, milliseconds{};
            std::size_t offsetFormat{}, offsetTime{};

            const auto fnNextChar = [](std::string_view format, std::size_t& offset) -> char {
                return offset < format.size() ? format[offset++] : char{};
            };

            const auto fnParse = [](std::string_view buffer, std::size_t& offset, std::size_t read_length, std::int32_t& out)
            {
                if(!out)
                {
                    // Take string with fixed size length from buffer
                    const auto data = buffer.substr(offset, read_length);

                    // Buffer can be small, need compare for check
                    HELENA_ASSERT(data.size() == read_length, "Parse data: \"{}\" failed, data size less than read_length", data);
                    if(data.size() == read_length)
                    {
                        // Cast string to int
                        const auto [ptr, err] = std::from_chars(data.data(), data.data() + data.size(), out);
                        HELENA_ASSERT(err == std::errc{}, "Cast data: \"{}\" failed", data);

                        // Check cast result
                        if(err == std::errc{}) {
                            // Add size to offset for parse other data in next time
                            offset += data.size();
                            return true;
                        }
                    }
                }

                return false;
            };


            while(true)
            {
                switch(char key = fnNextChar(format, offsetFormat))
                {
                    case '\0': return DateTime{DateToTicks(year, month, day) + TimeToTicks(hour, minutes, seconds, milliseconds)};
                    case '%': break;

                    case 'D': {
                        HELENA_ASSERT(!day);
                        if(!fnParse(time, offsetTime, 2uLL, day)) goto RETURN;
                    } break;

                    case 'M': {
                        HELENA_ASSERT(!month);
                        if(!fnParse(time, offsetTime, 2uLL, month)) goto RETURN;
                    } break;

                    case 'Y': {
                        HELENA_ASSERT(!year);
                        if(!fnParse(time, offsetTime, 4uLL, year)) goto RETURN;
                    } break;

                    case 'h': {
                        HELENA_ASSERT(!hour);
                        if(!fnParse(time, offsetTime, 2uLL, hour)) goto RETURN;
                    } break;

                    case 'm':
                    {
                        if((offsetFormat + 1) < format.size() && format[offsetFormat + 1] == 's') {
                            HELENA_ASSERT(!milliseconds);
                            fnNextChar(format, offsetFormat);
                            key = fnParse(time, offsetTime, 3uLL, milliseconds);
                        } else {
                            HELENA_ASSERT(!minutes);
                            key = fnParse(time, offsetTime, 2uLL, minutes);
                        }

                        if(!key) goto RETURN;
                    } break;

                    case 's': {
                        HELENA_ASSERT(!seconds);
                        if(!fnParse(time, offsetTime, 2uLL, seconds)) goto RETURN;
                    } break;

                    default: {
                        if(key != fnNextChar(time, offsetTime)) {
                            HELENA_ASSERT(key == fnNextChar(time, offsetTime),
                                "Format: \"{}\" and Time: \"{}\" separators do not match!", key, fnNextChar(time, offsetTime));
                            goto RETURN;
                        }
                    } break;
                }
            }

        RETURN:
            return DateTime{};
        }

        [[nodiscard]] static constexpr std::int32_t GetDaysInMonth(std::int32_t year, std::int32_t month) noexcept {
            HELENA_ASSERT(month >= 1 && month <= Months);
            return DaysPerMonth[month] - DaysPerMonth[month - 1uLL] + (month == 2 && IsLeapYear(year));
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
            HELENA_ASSERT(Valid(year, month, day));
            --year; --month; --day;
            return (year * 365LL + year / 4 - year / 100 + year / 400 + DaysPerMonth[month] + day + (month > 2 && IsLeapYear(year + 1))) * m_TicksPerDays;
        }

        [[nodiscard]] static constexpr std::int64_t TimeToTicks(std::int32_t hour, std::int32_t minute = 0,
                                                                std::int32_t second = 0, std::int32_t millisecond = 0) noexcept {
            HELENA_ASSERT(Valid(hour, minute, second, millisecond));
            return hour * m_TicksPerHours + minute * m_TicksPerMinutes + second * m_TicksPerSeconds + millisecond * m_TicksPerMilliseconds;
        }

        [[nodiscard]] static constexpr bool Valid(std::int32_t year, std::int32_t month, std::int32_t day, std::int32_t hour,
            std::int32_t minute, std::int32_t second, std::int32_t millisecond) noexcept {
            return Valid(year, month, day) && Valid(hour, minute, second, millisecond);
        }

        [[nodiscard]] static constexpr bool Valid(std::int32_t year, std::int32_t month, std::int32_t day) noexcept {
            return (year >= 1 && year <= 9999)
                && (month >= 1 && month <= Months)
                && (day >= 1 && day <= GetDaysInMonth(year, month));
        }

        [[nodiscard]] static constexpr bool Valid(std::int32_t hour, std::int32_t minute, std::int32_t second, std::int32_t millisecond) noexcept {
            return (hour >= 0 && hour <= 23)
                && (minute >= 0 && minute <= 59)
                && (second >= 0 && second <= 59)
                && (millisecond >= 0 && millisecond <= 999);
        }

        [[nodiscard]] constexpr bool IsNull() const noexcept {
            return !m_Ticks;
        }

        [[nodiscard]] constexpr bool IsNegative() const noexcept {
            return m_Ticks < 0;
        }

        constexpr void AddTicks(std::int64_t ticks) noexcept {
            m_Ticks += ticks;
        }

        constexpr void AddMilliseconds(std::int64_t milliseconds) noexcept {
            m_Ticks += milliseconds * m_TicksPerMilliseconds;
        }

        constexpr void AddSeconds(std::int64_t seconds) noexcept {
            m_Ticks += seconds * m_TicksPerSeconds;
        }

        constexpr void AddMinutes(std::int64_t minutes) noexcept {
            m_Ticks += minutes * m_TicksPerMinutes;
        }

        constexpr void AddHours(std::int64_t hours) noexcept {
            m_Ticks += hours * m_TicksPerHours;
        }

        constexpr void AddDays(std::int64_t days) noexcept {
            m_Ticks += days * m_TicksPerDays;
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
            std::int32_t year{};
            std::int32_t month{};
            std::int32_t day{};
            GetDate(year, month, day);
            return day;
        }

        [[nodiscard]] constexpr std::int32_t GetMonth() const noexcept {
            std::int32_t year{};
            std::int32_t month{};
            std::int32_t day{};
            GetDate(year, month, day);
            return month;
        }

        [[nodiscard]] constexpr std::int32_t GetYear() const noexcept {
            std::int32_t year{};
            std::int32_t month{};
            std::int32_t day{};
            GetDate(year, month, day);
            return year;
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

        [[nodiscard]] constexpr std::int64_t GetTimeStamp() const noexcept {
            return (m_Ticks - DateToTicks(1970, 1, 1)) / m_TicksPerSeconds;
        }

        [[nodiscard]] constexpr EDaysOfWeek GetDayOfWeek() const noexcept {
            return static_cast<EDaysOfWeek>((m_Ticks / m_TicksPerDays) % DaysInWeek);
        }

        [[nodiscard]] constexpr double GetJulianDay() const noexcept {
            return static_cast<double>(m_Ticks) / static_cast<double>(m_TicksPerDays) + 1721425.5;
        }

        [[nodiscard]] constexpr DateTime GetDate() const noexcept {
            return DateTime{m_Ticks - m_Ticks % m_TicksPerDays};
        }

        constexpr void GetDate(std::int32_t& year, std::int32_t& month, std::int32_t& day) const noexcept {
            std::int32_t l = static_cast<std::int32_t>(GetJulianDay() + 0.5) + 68569;
            std::int32_t n = 4 * l / 146097;
            l = l - (146097 * n + 3) / 4;
            std::int32_t i = 4000 * (l + 1) / 1461001;
            l = l - 1461 * i / 4 + 31;
            std::int32_t j = 80 * l / 2447;
            std::int32_t k = l - 2447 * j / 80;
            l = j / 11;
            j = j + 2 - 12 * l;
            i = 100 * (n - 49) + i + l;
            year = i; month = j; day = k;
        }


        [[nodiscard]] constexpr DateTime operator+(const DateTime other) const noexcept {
            return DateTime{m_Ticks + other.m_Ticks};
        }

        [[nodiscard]] constexpr DateTime operator-(const DateTime other) const noexcept {
            return DateTime{m_Ticks - other.m_Ticks};
        }

        [[nodiscard]] constexpr DateTime operator*(const DateTime other) const noexcept {
            return DateTime{m_Ticks * other.m_Ticks};
        }

        [[nodiscard]] constexpr DateTime operator/(const DateTime other) const noexcept {
            HELENA_ASSERT(other.m_Ticks, "Divide by zero");
            return DateTime{m_Ticks / other.m_Ticks};
        }

        [[nodiscard]] constexpr DateTime& operator+=(const DateTime other) noexcept {
            m_Ticks += other.m_Ticks;
            return *this;
        }

        [[nodiscard]] constexpr DateTime& operator-=(const DateTime other) noexcept {
            m_Ticks -= other.m_Ticks;
            return *this;
        }

        [[nodiscard]] constexpr DateTime& operator*=(const DateTime other) noexcept {
            m_Ticks *= other.m_Ticks;
            return *this;
        }

        [[nodiscard]] constexpr DateTime& operator/=(const DateTime other) noexcept {
            HELENA_ASSERT(other.m_Ticks, "Divide by zero");
            m_Ticks /= other.m_Ticks;
            return *this;
        }

        [[nodiscard]] constexpr auto operator<=>(const DateTime&) const noexcept = default;

    private:
        std::int64_t m_Ticks;
    };
}

#endif // HELENA_TYPES_DATETIME_HPP