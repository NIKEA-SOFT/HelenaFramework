#ifndef HELENA_ENGINE_LOG_HPP
#define HELENA_ENGINE_LOG_HPP

#include <Helena/Dependencies/Fmt.hpp>
#include <Helena/Types/SourceLocation.hpp>

namespace Helena::Log
{
    namespace Details {
        template <typename T>
        concept Prefixable = requires {
            //{ T::is_logging }   -> Traits::AnyOf<std::true_type, std::false_type>;
            { T::GetPrefix() }  -> std::same_as<std::string_view>;
            { T::GetStyle() }   -> std::same_as<fmt::text_style>;
        };
    }

    enum class Color : std::uint8_t;

    [[nodiscard]] inline consteval auto CreatePrefix(const std::string_view prefix) noexcept;

    [[nodiscard]] inline consteval auto CreateStyle(const Color color) noexcept;

    [[nodiscard]] inline consteval auto CreateStyle(const Color color, const Color background) noexcept;

    template <Details::Prefixable Prefix, typename... Args>
    void Console(const Types::SourceLocation& source, const std::string_view msg, Args&&... args) noexcept;
}

#include <Helena/Engine/Log.ipp>

// NOTE MSVC: If you get an error, make sure you add a preprocessor /Zc:preprocessor for support VA_OPT
// Read it here: https://docs.microsoft.com/en-us/cpp/build/reference/zc-preprocessor?view=msvc-160
#define HELENA_MSG(prefix, fmt, ...)        Helena::Log::Console<prefix>(Helena::Types::SourceLocation::Create(__FILE__, __LINE__), fmt __VA_OPT__(,) __VA_ARGS__)

#if defined(HELENA_DEBUG)
    #define HELENA_MSG_DEFAULT(fmt, ...)    HELENA_MSG(Helena::Log::Details::Default,   fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_DEBUG(fmt, ...)      HELENA_MSG(Helena::Log::Details::Debug,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_INFO(fmt, ...)       HELENA_MSG(Helena::Log::Details::Info,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_WARNING(fmt, ...)    HELENA_MSG(Helena::Log::Details::Warning,   fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ERROR(fmt, ...)      HELENA_MSG(Helena::Log::Details::Error,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_FATAL(fmt, ...)      HELENA_MSG(Helena::Log::Details::Fatal,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ASSERT(fmt, ...)     HELENA_MSG(Helena::Log::Details::Assert,    fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_EXCEPTION(fmt, ...)  HELENA_MSG(Helena::Log::Details::Exception, fmt __VA_OPT__(,) __VA_ARGS__)
#else
    #define HELENA_MSG_DEFAULT(fmt, ...)
    #define HELENA_MSG_DEBUG(fmt, ...)
    #define HELENA_MSG_INFO(fmt, ...)
    #define HELENA_MSG_WARNING(fmt, ...)    HELENA_MSG(Helena::Log::Details::Warning,   fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ERROR(fmt, ...)      HELENA_MSG(Helena::Log::Details::Error,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_FATAL(fmt, ...)      HELENA_MSG(Helena::Log::Details::Fatal,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ASSERT(fmt, ...)     HELENA_MSG(Helena::Log::Details::Assert,    fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_EXCEPTION(fmt, ...)  HELENA_MSG(Helena::Log::Details::Exception, fmt __VA_OPT__(,) __VA_ARGS__)
#endif


#endif // HELENA_ENGINE_LOG_HPP
