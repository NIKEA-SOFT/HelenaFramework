#ifndef HELENA_ENGINE_LOG_HPP
#define HELENA_ENGINE_LOG_HPP

#include <Helena/Types/BasicLogger.hpp>

namespace Helena::Log
{
    struct Debug {
        using Logger = Types::BasicLogger;

        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return Logger::CreatePrefix("[DEBUG]");
        }

        [[nodiscard]] static consteval auto GetStyle() noexcept {
            return Logger::CreateStyle(Logger::Color::BrightBlue);
        }
    };

    struct Info {
        using Logger = Types::BasicLogger;

        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return Logger::CreatePrefix("[INFO]");
        }

        [[nodiscard]] static consteval auto GetStyle() noexcept {
            return Logger::CreateStyle(Logger::Color::BrightGreen);
        }
    };

    struct Notice {
        using Logger = Types::BasicLogger;

        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return Logger::CreatePrefix("[NOTICE]");
        }

        [[nodiscard]] static consteval auto GetStyle() noexcept {
            return Logger::CreateStyle(Logger::Color::BrightWhite);
        }
    };

    struct Warning {
        using Logger = Types::BasicLogger;

        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return Logger::CreatePrefix("[WARNING]");
        }

        [[nodiscard]] static consteval auto GetStyle() noexcept {
            return Logger::CreateStyle(Logger::Color::BrightYellow);
        }
    };

    struct Error {
        using Logger = Types::BasicLogger;

        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return Logger::CreatePrefix("[ERROR]");
        }

        [[nodiscard]] static consteval auto GetStyle() noexcept {
            return Logger::CreateStyle(Logger::Color::BrightRed);
        }
    };

    struct Fatal {
        using Logger = Types::BasicLogger;

        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return Logger::CreatePrefix("[FATAL]");
        }

        [[nodiscard]] static consteval auto GetStyle() noexcept {
            return Logger::CreateStyle(Logger::Color::BrightWhite, Logger::Color::Red);
        }
    };

    template <Traits::DefinitionLogger Logger, typename... Args>
    void Console(const Types::BasicLogger::Formater<Logger> format, Args&&... args) noexcept {
        Types::BasicLogger{format, std::forward<Args>(args)...};
    }

    template <typename... Args>
    void Console(const Traits::DefinitionLogger auto logger, const Types::BasicLogger::Formater<std::decay_t<decltype(logger)>> format, Args&&... args) noexcept {
        Types::BasicLogger{format, std::forward<Args>(args)...};
    }
}

// NOTE MSVC: If you get an error, make sure you add a preprocessor /Zc:preprocessor for support VA_OPT
// Read it here: https://docs.microsoft.com/en-us/cpp/build/reference/zc-preprocessor?view=msvc-160
#define HELENA_MSG(prefix, fmt, ...)        Helena::Log::Console<prefix>(fmt __VA_OPT__(,) __VA_ARGS__)

#if defined(HELENA_DEBUG)
    #define HELENA_MSG_DEBUG(fmt, ...)      HELENA_MSG(Helena::Log::Debug,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_INFO(fmt, ...)       HELENA_MSG(Helena::Log::Info,       fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_NOTICE(fmt, ...)     HELENA_MSG(Helena::Log::Notice,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_WARNING(fmt, ...)    HELENA_MSG(Helena::Log::Warning,    fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ERROR(fmt, ...)      HELENA_MSG(Helena::Log::Error,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_FATAL(fmt, ...)      HELENA_MSG(Helena::Log::Fatal,      fmt __VA_OPT__(,) __VA_ARGS__)
#else
    #define HELENA_MSG_DEBUG(fmt, ...)
    #define HELENA_MSG_INFO(fmt, ...)
    #define HELENA_MSG_NOTICE(fmt, ...)     HELENA_MSG(Helena::Log::Notice,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_WARNING(fmt, ...)    HELENA_MSG(Helena::Log::Warning,    fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ERROR(fmt, ...)      HELENA_MSG(Helena::Log::Error,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_FATAL(fmt, ...)      HELENA_MSG(Helena::Log::Fatal,      fmt __VA_OPT__(,) __VA_ARGS__)
#endif


#endif // HELENA_ENGINE_LOG_HPP
