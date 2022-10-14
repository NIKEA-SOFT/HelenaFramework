#ifndef HELENA_TYPES_BASICLOGGER_HPP
#define HELENA_TYPES_BASICLOGGER_HPP

#include <Helena/Types/BasicLoggersDef.hpp>

namespace Helena::Types
{
    class BasicLogger
    {
        [[nodiscard]] static bool MakeColor(fmt::memory_buffer& buffer, const fmt::text_style style);
        static void EndColor(fmt::memory_buffer& buffer) noexcept;

    public:
        BasicLogger() = delete;
        ~BasicLogger() = delete;
        BasicLogger(const BasicLogger&) = delete;
        BasicLogger(BasicLogger&&) noexcept = delete;
        BasicLogger& operator=(const BasicLogger&) = delete;
        BasicLogger& operator=(BasicLogger&&) noexcept = delete;

        template <Traits::DefinitionLogger Logger, typename... Args>
        static void PrintConsole(const Log::Formater<Logger> format, [[maybe_unused]] Args&&... args);
    };
}

#include <Helena/Types/BasicLogger.ipp>

#endif // HELENA_TYPES_BASICLOGGER_HPP