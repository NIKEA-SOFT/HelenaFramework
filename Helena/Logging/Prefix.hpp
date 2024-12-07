#ifndef HELENA_LOGGING_PREFIX_HPP
#define HELENA_LOGGING_PREFIX_HPP

#include <Helena/Logging/ColorStyle.hpp>

namespace Helena::Logging
{
    // Structures defining the type and color of the logged message
    struct Debug {
        static constexpr auto Prefix = CreatePrefix("DEBUG");
        static constexpr auto Style = CreateStyle(Color::BrightBlue);
    };

    struct Info {
        static constexpr auto Prefix = CreatePrefix("INFO");
        static constexpr auto Style = CreateStyle(Color::BrightGreen);
    };

    struct Notice {
        static constexpr auto Prefix = CreatePrefix("NOTICE");
        static constexpr auto Style = CreateStyle(Color::BrightWhite);
    };

    struct Warning {
        static constexpr auto Prefix = CreatePrefix("WARNING");
        static constexpr auto Style = CreateStyle(Color::BrightYellow);
    };

    struct Error {
        static constexpr auto Prefix = CreatePrefix("ERROR");
        static constexpr auto Style = CreateStyle(Color::BrightRed);
    };

    struct Fatal {
        static constexpr auto Prefix = CreatePrefix("FATAL");
        static constexpr auto Style = CreateStyle(Color::BrightWhite, Color::Red);
    };

    struct Assert {
        static constexpr auto Prefix = CreatePrefix("ASSERT");
        static constexpr auto Style = CreateStyle(Color::BrightWhite, Color::Red);
    };

    struct Exception {
        static constexpr auto Prefix = CreatePrefix("EXCEPTION");
        static constexpr auto Style = CreateStyle(Color::BrightWhite, Color::Red);
    };

    struct Memory {
        static constexpr auto Prefix = CreatePrefix("MEMORY");
        static constexpr auto Style = CreateStyle(Color::BrightCyan);
    };

    struct Benchmark {
        static constexpr auto Prefix = CreatePrefix("BENCHMARK");
        static constexpr auto Style = CreateStyle(Color::BrightMagenta);
    };

    struct Shutdown {
        static constexpr auto Prefix = CreatePrefix("SHUTDOWN");
        static constexpr auto Style = CreateStyle(Color::BrightWhite, Color::Red);
    };
}

#endif // HELENA_LOGGING_PREFIX_HPP
